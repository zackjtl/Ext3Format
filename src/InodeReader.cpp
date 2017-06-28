//---------------------------------------------------------------------------
#include "Pch.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "InodeReader.h"
#include "FolderInodeReader.h"
#include "GlobalDef.h"
#include "BaseError.h"
#include "fs_assert.h"
//---------------------------------------------------------------------------
CInodeReader* CInodeReader::Create(TInode* Inode, uint16 BlockSize)
{
  uint type = type = DecToOct(Inode->Mode & 0xF000);

	if (LINUX_S_IFDIR == type) {
		return new CFolderInodeReader(Inode, BlockSize);
	}
	else if (LINUX_S_IFREG == type) {
 		return new CInodeReader(Inode, BlockSize);
	}
  else {
    return new CInodeReader(Inode, BlockSize);
  }
}

CInodeReader::CInodeReader(TInode* InodeIn, uint16 BlockSize)
  : CInode(0, BlockSize)
{
  Inode = *InodeIn;

  Type = DecToOct(Inode.Mode & 0xF000);
  Permissions = DecToOct(Inode.Mode & 0x0FFF);
}

CInodeReader::~CInodeReader()
{
}

bool CInodeReader::IsFolderInode()
{
  return LINUX_S_IFDIR == Type ? true : false;
}

Bulk<byte>& CInodeReader::GetDataBuffer()
{
  return _DataBuffer;
}

/*
 *  For regular file inodes, only rebuild the blocks link.
 */
void CInodeReader::Read(CBlockManager& BlockMan, CUsbDrive* Drive)
{
  RebuildInodeBlocks(BlockMan, Drive);
}

/*
 *  Rebuild inode direct and indirect blocks in the data from media device.
 */
void CInodeReader::RebuildInodeBlocks(CBlockManager& BlockMan, CUsbDrive* Drive)
{
  uint32 sectorsPerBlock = _BlockSize / 512;
  uint32 totalBlocks = Inode.SectorCount / sectorsPerBlock;

  uint32 remainBlocks = totalBlocks;

  ////remainBlocks -= ReadDirectBlocks(BlockMan, Drive, ptr, remainBlocks);
  uint directBlocks = min_of(EXT2_NDIR_BLOCKS, remainBlocks);

  for (uint idx = 0; idx < directBlocks; ++idx) {
    Direct.push_back(Inode.Blocks[idx]);
  }
  remainBlocks -= directBlocks;

  if (remainBlocks) {
    remainBlocks -= RebuildIndirectBlocks(BlockMan, Drive, remainBlocks);

    if (remainBlocks) {
      remainBlocks -= RebuildMultiIndirectBlocks(BlockMan, Drive, 2, remainBlocks);

      if (remainBlocks) {
        remainBlocks -= RebuildMultiIndirectBlocks(BlockMan, Drive, 3, remainBlocks);
      }
    }
  }
}

/*
 *  Rebuild indirct block address link
 */
uint32 CInodeReader::RebuildIndirectBlocks(CBlockManager& BlockMan, CUsbDrive* Drive, uint32 RemainBlocks)
{
  uint32 capability = AddrPerBlock;
  uint32 readCnt = min_of(RemainBlocks, capability);

  Bulk<byte>  addrTable(_BlockSize);

  if (Drive != NULL) {
    Bulk<byte>* buffer = LoadSingleBlockDataFromMedia(BlockMan, Drive, Inode.Blocks[EXT2_IND_BLOCK]);
    AddBlockArray(&Indirect, buffer);
  }
  return readCnt;
}

/*
 *  Rebuild double and tripple indirect block links from media device
 */
uint32 CInodeReader::RebuildMultiIndirectBlocks(CBlockManager& BlockMan, CUsbDrive* Drive,
                              uint32 Layer, uint32 RemainBlocks)
{
  assert(Layer == 2 || Layer == 3);
  uint32 rootBlock = Layer == 2 ? Inode.Blocks[EXT2_DIND_BLOCK] : Inode.Blocks[EXT2_TIND_BLOCK];
  CIndrMatrix* matrix = Layer == 2 ? &DIndirect : &TIndirect;

  uint32 leafLayer = Layer - 1;
  uint32 layer_max_size[3];
  uint32 layer_needed_size[3];
  make_layer_max_sizes(layer_max_size);

  uint32 capability = layer_max_size[leafLayer];
  uint32 child_cnt = capability;
  uint32 readCnt = min_of(RemainBlocks, child_cnt);

  layer_needed_size[leafLayer] = readCnt;

  for (uint32 layer = (leafLayer - 1); layer >= 0; --layer) {
    layer_needed_size[layer] = div_ceil(layer_needed_size[layer + 1], AddrPerBlock);
  }

  vector<uint32>* parent_array = &(*matrix)[0];

  if (Drive != NULL) {
    Bulk<byte>* buffer = LoadSingleBlockDataFromMedia(BlockMan, Drive, rootBlock);
    AddBlockArray(parent_array, buffer);
  }
  for (uint32 layer = 1; layer < Layer; ++layer) {
    uint32 count = parent_array->size();

    vector<uint32>* child_array = &(*matrix)[layer];

    for (uint32 idx = 0; idx < count; ++idx) {      Bulk<byte>* buffer = LoadSingleBlockDataFromMedia(BlockMan, Drive, (*parent_array)[idx]);
      AddBlockArray(child_array, buffer);
    }
    parent_array = &(*matrix)[layer];
  }
}

/*
 *  Add blocks into array from the buffer data
 */
void CInodeReader::AddBlockArray(vector<uint32>* Array, Bulk<byte>* Buffer)
{
  uint32 newCount = Buffer->Size() / sizeof(uint32);
  Array->insert(Array->begin(), (uint32*)Buffer->Data(), (uint32*)(Buffer->Data() + newCount));
}


/*
 *  Read block data from media to buffer
 */
Bulk<byte>* CInodeReader::LoadSingleBlockDataFromMedia(CBlockManager& BlockMan, CUsbDrive* Drive, uint32 Block)
{
  if (!BlockMan.HasBlockOccupied(Block)) {
    ////return 0;
    throw CError(L"The rebuilt block was set as free in the block bitmap");
  }
  if (Drive == NULL) {
    throw CError(L"Can't get data from a non existing block");
  }
  ////BlockMan.OccupyBlock(Block, 1);

  Bulk<byte>* buffer = BlockMan.CreateSingleBlockDataBuffer(Block);

  assert(buffer->Size() >= (_BlockSize));

  uint32 sectorsPerBlock = _BlockSize / 512;
  Drive->ReadSector(Block * sectorsPerBlock, sectorsPerBlock, buffer->Data());

  return buffer;
}
