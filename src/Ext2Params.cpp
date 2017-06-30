#include "Ext2Params.h"
#include "e3fs_tables.h"
#include "e3fs_def.h"

/*
 *  Total Sectors: The capacity of the storage media.
 *  Block Szie: Bytes per block (2k, 4k or 8k)
 */
CExt2Params::CExt2Params(uint64 TotalSectors)
{
  MBRSize = 1024;
  BlockSize = 4096;
  InodeSize = 256;
  BlocksPerGroup = 32768;
  InodeRatio = 16384;
  TimeOfResize = 1024;
  ReservedPercent = 5;

  TotalBlocks = (uint64)TotalSectors / (BlockSize / 512);

  uint32 descPerBlock = BlockSize  / sizeof(TGroupDesc);
 
  GroupCount = div_ceil(TotalBlocks, BlocksPerGroup);
 
  uint32 group_desc_size = sizeof(TGroupDesc); 

  GroupDescPerBlock = BlockSize / group_desc_size;
  GroupDescBlockCnt = div_ceil(GroupCount, GroupDescPerBlock);    

  BlocksOfLastGroup = TotalBlocks % BlocksPerGroup;
  RsvdGdtBlockOffset = 1 + GroupDescBlockCnt;
}

/*
 *  The default constructor is used in loading parameters from filesystem.
 */

CExt2Params::CExt2Params()
{
  MBRSize = 1024;
  ////BlockSize = 4096;
  ////InodeSize = 256;
  ////BlocksPerGroup = 32768;
  InodeRatio = 16384;
  TimeOfResize = 1024;
  ReservedPercent = 5;
}

void CExt2Params::CompleteParameters()
{
  uint32 group_desc_size = sizeof(TGroupDesc); 

  GroupDescPerBlock = BlockSize / group_desc_size;
  GroupDescBlockCnt = div_ceil(GroupCount, GroupDescPerBlock);    

  BlocksOfLastGroup = TotalBlocks % BlocksPerGroup;
  RsvdGdtBlockOffset = 1 + GroupDescBlockCnt;
}

CExt2Params::~CExt2Params()
{
}