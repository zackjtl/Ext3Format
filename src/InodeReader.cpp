//---------------------------------------------------------------------------
#include "Pch.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "InodeReader.h"
#include "FolderInodeReader.h"
#include "GlobalDef.h"
#include "BaseError.h"
#include "E2fsBlockIo.h"
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

  DIndirect.resize(2);
  TIndirect.resize(3);  
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
  uint64 inodeSize = Inode.SizeInBytesLo + ((uint64)Inode.SizeInBytesHi << 32);
  uint32 totalBlocks = div_ceil(inodeSize, _BlockSize);

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
  uint32 capability = _AddrPerBlock;
  uint32 readCnt = min_of(RemainBlocks, capability);

  Bulk<byte>  addrTable(_BlockSize);

  if (Drive != NULL) {     
    Bulk<byte>* buffer = LoadSingleBlockDataFromMedia(BlockMan, Drive, Inode.Blocks[EXT2_IND_BLOCK]);
    AddBlockArray(&Indirect, buffer, readCnt);    
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

  for (int layer = (leafLayer - 1); layer >= 0; --layer) {
    layer_needed_size[layer] = div_ceil(layer_needed_size[layer + 1], _AddrPerBlock);
  }

  vector<uint32>* parent_array = &(*matrix)[0];

  if (Drive != NULL) {
    Bulk<byte>* buffer = LoadSingleBlockDataFromMedia(BlockMan, Drive, rootBlock);
    AddBlockArray(parent_array, buffer, layer_needed_size[0]);
  }
  for (uint32 layer = 1; layer < Layer; ++layer) {
    uint32 count = parent_array->size();
    uint32 child_remain = layer_needed_size[layer];

    vector<uint32>* child_array = &(*matrix)[layer];

    if (Drive != NULL) {
      CE2fsBlockIo io(BlockMan, Drive, true);
      io.SetArrayToQueue(*parent_array);
      io.ReadBlocksInQueue();
    }

    for (uint32 idx = 0; idx < count; ++idx) {
      uint32 add_cnt = min_of(child_remain, _AddrPerBlock);      
      Bulk<byte>* buffer = BlockMan.GetSingleBlockDataBuffer((*parent_array)[idx]);
      AddBlockArray(child_array, buffer, add_cnt);
      child_remain -= add_cnt;
    }
    assert(child_array->size() == layer_needed_size[layer]);
    assert(child_remain == 0);
    parent_array = &(*matrix)[layer];
  }
  return readCnt;
}

/*
 *  Add blocks into array from the buffer data
 */
void CInodeReader::AddBlockArray(vector<uint32>* Array, Bulk<byte>* Buffer, uint32 Count)
{
  uint32 maxCnt = Buffer->Size() / sizeof(uint32);
  fs_assert_true(maxCnt >= Count);

  uint32 size = Count * sizeof(uint32);

  Array->insert(Array->end(), (uint32*)Buffer->Data(), (uint32*)(Buffer->Data() + size));
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

/*
 * Calculate leaf blocks from the block link trees
 */
uint32 CInodeReader::GetTotalDataBlocks()
{
  return Direct.size() + Indirect.size() +
          DIndirect[1].size() + TIndirect[2].size();
}

/*
 *  Rebuild inode whole data blocks.
 */
void CInodeReader::ReadInodeData(CBlockManager& BlockMan, CUsbDrive* Drive, uint32 ReadBlocks)
{
  uint32 sectorsPerBlock = _BlockSize / 512;
  uint64 inodeSize = Inode.SizeInBytesLo + ((uint64)Inode.SizeInBytesHi << 32);
  uint32 expectedBlocks = div_ceil(inodeSize, _BlockSize);
  uint32 totalBlocks = GetTotalDataBlocks();

  fs_assert_eq(expectedBlocks, totalBlocks);

  _DataBuffer.Resize(totalBlocks * _BlockSize);

  byte* ptr = _DataBuffer.Data();

  uint32 totalReaded = 0;
  uint32 targetSize = ReadBlocks == 0 ? _DataBuffer.Size() :
                        min_of(ReadBlocks, totalBlocks) * _BlockSize;

  uint32 remainBuffer = _DataBuffer.Size();

  uint32 readSize = ReadDirectBlocks(BlockMan, Drive, ptr, remainBuffer);
  remainBuffer -= readSize;
  ptr += readSize;
  totalReaded += readSize;

  if (remainBuffer && (totalReaded < targetSize)) {
    readSize = ReadIndirectLeafBlocks(BlockMan, Drive, ptr, remainBuffer);
    remainBuffer -= readSize;
    ptr += readSize;
    totalReaded += readSize;

    if (remainBuffer && (totalReaded < targetSize)) {
      readSize = ReadMultiIndirectLeafBlocks(BlockMan ,Drive, 2, ptr, remainBuffer);
      remainBuffer -= readSize;
      ptr += readSize;
      totalReaded += readSize;

      if (remainBuffer && (totalReaded < targetSize)) {
        readSize = ReadMultiIndirectLeafBlocks(BlockMan ,Drive, 3, ptr, remainBuffer);
        remainBuffer -= readSize;
        totalReaded += readSize;
      }
    }
  }
  if (ReadBlocks == 0) {
    fs_assert_eq(remainBuffer, 0);
  }
  else {
    fs_assert_true(totalReaded >= targetSize);
  }
}

/*
 *  When the pointer to Drive is null, this funciton is used for geting whole block data
 *   from block manager that has been setted in earlier.
 *   Otherwise, this is used to read data from media device.
 */
uint32 CInodeReader::ReadDirectBlocks(CBlockManager& BlockMan, CUsbDrive* Drive,
                                      byte* Buffer, uint32 Length)
{
  uint32 blocks = Direct.size();
  byte* ptr = Buffer;

  if (Length < (blocks * _BlockSize)) {
    throw CError(L"Insuffcient buffer length to read direct blocks data");
  }
  if (Drive != NULL) {
    CE2fsBlockIo io(BlockMan, Drive, true);
    io.SetArrayToQueue(Direct);
    io.ReadBlocksInQueue();
  }  

  for (uint32 block = 0; block < blocks; ++block) {
    uint32 realBlock = Inode.Blocks[block];
    BlockMan.GetSingleBlockData(realBlock, ptr, _BlockSize);    
    ptr += _BlockSize;
  }  
  return blocks * _BlockSize;
}

uint32 CInodeReader::ReadIndirectLeafBlocks(CBlockManager& BlockMan, CUsbDrive* Drive,
                            byte* Buffer, uint32 Length)
{
  uint32 blockCnt = Indirect.size();
  byte* ptr = Buffer;
  
  if (Length < (blockCnt * _BlockSize)) {
    throw CError(L"Insuffcient buffer length to read indirect blocks data");
  }

  if (Drive != NULL) {
    CE2fsBlockIo io(BlockMan, Drive, true);
    io.SetArrayToQueue(Indirect);
    io.ReadBlocksInQueue();
  }
  
  for (uint32 block = 0; block < blockCnt; ++block) {
    uint32 realBlock = Indirect[block];
    BlockMan.GetSingleBlockData(realBlock, ptr, _BlockSize);    
    ptr += _BlockSize;
  }  
  return blockCnt * _BlockSize;
}

/*
 *  Read double and tripple indirect block data from media device or from
 *   block manager.
 */
uint32 CInodeReader::ReadMultiIndirectLeafBlocks(CBlockManager& BlockMan, CUsbDrive* Drive,
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
  if (Drive != NULL) {
    CE2fsBlockIo io(BlockMan, Drive, true);
    io.SetArrayToQueue((*matrix)[leafLayer]);
    io.ReadBlocksInQueue();
  }
  
  for (uint32 block = 0; block < leafBlocks; ++block) {
    uint32 realBlock = (*matrix)[leafLayer][block];     
    BlockMan.GetSingleBlockData(realBlock, ptr, _BlockSize);
    ptr += _BlockSize;
  }
  return leafBlocks * _BlockSize;
}

