#include "Ext2Params.h"
#include "Tables.h"

uint32 CExt2Params::BlockSize = 4096;
uint32 CExt2Params::InodeSize = 256;
uint32 CExt2Params::BlocksPerGroup = 32768;
uint32 CExt2Params::InodeRatio = 4;
uint32 CExt2Params::TimeOfResize = 1024;

/*
 *  Total Sectors: The capacity of the storage media.
 *  Block Szie: Bytes per block (2k, 4k or 8k)
 */
CExt2Params::CExt2Params(uint32 TotalSectors)
  : _TotalSectors(TotalSectors)
{
  uint32 descPerBlock = BlockSize / sizeof(TGroupDesc);
  uint32 descPerGroup = descPerBlock * BlocksPerGroup;

  _TotalBlocks = _TotalSectors / BlockSize;
  _GroupCount = _TotalBlocks / BlocksPerGroup;
  _InodesPerGroup = ((_TotalBlocks / 4) + _GroupCount - 1) / _GroupCount;
  _TotalInodes = _InodesPerGroup * _GroupCount;
  _GdtReservedBlocks = ((uint64)_TotalBlocks * TimeOfResize) / descPerGroup;
  _DescBlockCount = (_GroupCount + descPerBlock - 1) / descPerBlock;
}

CExt2Params::~CExt2Params()
{
}

