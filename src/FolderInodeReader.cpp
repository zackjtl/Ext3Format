//---------------------------------------------------------------------------
#include "Pch.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "FolderInodeReader.h"
#include "Tables.h"
#include "fs_assert.h"
//---------------------------------------------------------------------------
CFolderInodeReader::CFolderInodeReader(TInode* InodeIn, uint16 BlockSize)
  : CInodeReader(InodeIn, BlockSize)
{
  DIndirect.resize(2);
  TIndirect.resize(3);
  DotInode = 0;
  TwoDotInode = 0;
}

CFolderInodeReader::~CFolderInodeReader()
{

}

void CFolderInodeReader::AddEntry(ext2_dir_entry& Entry)
{
  std::string name = Entry.name;
  _NameList.insert(make_pair(name, Entry.inode));
}


/*
 *  For directory inodes, read all inode data to rebuild entries.
 */
void CFolderInodeReader::Read(CBlockManager& BlockMan, CUsbDrive* Drive)
{
  RebuildInodeBlocks(BlockMan, Drive);
  ReadInodeData(BlockMan, Drive);
}

/*
 *  Rebuild inode whole data blocks.
 */
void CFolderInodeReader::ReadInodeData(CBlockManager& BlockMan, CUsbDrive* Drive)
{
  uint32 sectorsPerBlock = _BlockSize / 512;
  uint32 expectedBlocks = Inode.SectorCount / sectorsPerBlock;
  uint32 totalBlocks = Direct.size() + Indirect.size() +
                       DIndirect[1].size() + TIndirect[2].size();

  fs_assert_eq(expectedBlocks, totalBlocks);

  _DataBuffer.Resize(totalBlocks * _BlockSize);

  byte* ptr = _DataBuffer.Data();
  uint32 remainSize = _DataBuffer.Size();

  uint32 readSize = ReadDirectBlocks(BlockMan, Drive, ptr, remainSize);
  remainSize -= readSize;

  if (remainSize) {
    readSize = ReadIndirectLeafBlocks(BlockMan, Drive, ptr, remainSize);
    remainSize -= readSize;

    if (remainSize) {
      readSize = ReadMultiIndirectLeafBlocks(BlockMan ,Drive, 2, ptr, remainSize);
      remainSize -= readSize;

      if (remainSize) {
        readSize = ReadMultiIndirectLeafBlocks(BlockMan ,Drive, 3, ptr, remainSize);
        remainSize -= readSize;
      }
    }
  }
  fs_assert_eq(remainSize, 0);
}

/*
 *  When the pointer to Drive is null, this funciton is used for geting whole block data
 *   from block manager that has been setted in earlier.
 *   Otherwise, this is used to read data from media device.
 */
uint32 CFolderInodeReader::ReadDirectBlocks(CBlockManager& BlockMan, CUsbDrive* Drive,
                                            byte* Buffer, uint32 Length)
{
  uint32 blocks = Direct.size();
  byte* ptr = Buffer;

  if (Length < (blocks * _BlockSize)) {
    throw CError(L"Insuffcient buffer length to read direct blocks data");
  }
  for (uint32 block = 0; block < blocks; ++block) {
    uint32 realBlock = Inode.Blocks[block];

    if (Drive != NULL) {
      LoadSingleBlockDataFromMedia(BlockMan, Drive, realBlock);
    }
    BlockMan.GetSingleBlockData(realBlock, ptr, _BlockSize);

    ptr += _BlockSize;
  }
  return blocks * _BlockSize;
}

uint32 CFolderInodeReader::ReadIndirectLeafBlocks(CBlockManager& BlockMan, CUsbDrive* Drive,
                            byte* Buffer, uint32 Length)
{
  uint32 blockCnt = Indirect.size();
  byte* ptr = Buffer;

  if (Length < (blockCnt * _BlockSize)) {
    throw CError(L"Insuffcient buffer length to read indirect blocks data");
  }

  for (uint32 block = 0; block < blockCnt; ++block) {
    uint32 realBlock = Indirect[block];

    if (Drive != NULL) {
      LoadSingleBlockDataFromMedia(BlockMan, Drive, realBlock);
    }
    BlockMan.GetSingleBlockData(realBlock, ptr, _BlockSize);

    ptr += _BlockSize;
  }
  return blockCnt * _BlockSize;
}

/*
 *  Read double and tripple indirect block data from media device or from
 *   block manager.
 */
uint32 CFolderInodeReader::ReadMultiIndirectLeafBlocks(CBlockManager& BlockMan, CUsbDrive* Drive,
                              uint32 Layer, byte* Buffer, uint32 Length)
{
  assert(Layer == 2 || Layer == 3);
  uint32 rootBlock = Layer == 2 ? Inode.Blocks[EXT2_DIND_BLOCK] : Inode.Blocks[EXT2_TIND_BLOCK];
  CIndrMatrix* matrix = Layer == 2 ? &DIndirect : &TIndirect;
  byte* ptr = Buffer;

  uint32 leafLayer = Layer - 1;
  uint32 layer_max_size[3];
  uint32 layer_curr_size[3];

  make_layer_curr_sizes(layer_curr_size, *matrix);
  make_layer_max_sizes(layer_max_size);

  uint leafBlocks = (*matrix)[leafLayer].size();

  if (Length < (leafBlocks * _BlockSize)) {
    throw CError(L"Insuffcient buffer length to multi-indirect blocks data");
  }
  for (uint32 block = 0; block < leafBlocks; ++block) {
    uint32 realBlock = (*matrix)[leafLayer][block];

    if (Drive != NULL) {
      LoadSingleBlockDataFromMedia(BlockMan, Drive, realBlock);
    }
    BlockMan.GetSingleBlockData(realBlock, ptr, _BlockSize);

    ptr += _BlockSize;
  }
  return leafBlocks * _BlockSize;
}
