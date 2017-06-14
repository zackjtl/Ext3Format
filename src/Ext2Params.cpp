#include "Ext2Params.h"
#include "Tables.h"
#include "GlobalDef.h"

uint32 CExt2Params::MBRSize = 1024;
uint32 CExt2Params::BlockSize = 4096;
uint32 CExt2Params::InodeSize = 256;
uint32 CExt2Params::BlocksPerGroup = 32768;
uint32 CExt2Params::InodeRatio = 16384;
uint32 CExt2Params::TimeOfResize = 1024;
uint8  CExt2Params::ReservedPercent = 5;

/*
 *  Total Sectors: The capacity of the storage media.
 *  Block Szie: Bytes per block (2k, 4k or 8k)
 */
CExt2Params::CExt2Params(uint64 TotalSectors)
{
  TotalBlocks = (uint64)TotalSectors / (BlockSize / 512);

  uint32 descPerBlock = BlockSize  / sizeof(TGroupDesc);
 
  GroupCount = div_ceil(TotalBlocks, BlocksPerGroup);
 
  uint32 group_desc_size = sizeof(TGroupDesc); 

  GroupDescPerBlock = BlockSize / group_desc_size;
  GroupDescBlockCnt = div_ceil(GroupCount, GroupDescPerBlock);    

  BlocksOfLastGroup = TotalBlocks % BlocksPerGroup;
  RsvdGdtBlockOffset = 1 + GroupDescBlockCnt;
}

CExt2Params::~CExt2Params()
{
}