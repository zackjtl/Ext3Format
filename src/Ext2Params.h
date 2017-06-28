#ifndef Ext2ParamsH
#define Ext2ParamsH

#include "BaseTypes.h"
#include "Tables.h"
#include <vector>

using namespace std;

class CExt2Params
{
public:
  CExt2Params();
  CExt2Params(uint64 TotalSectors);
  ~CExt2Params();

  void CompleteParameters();

public:
	uint32 MBRSize;
  uint16 BlockSize;
  uint32 InodeSize;
  uint32 BlocksPerGroup;
  uint32 InodeRatio;
  uint32 TimeOfResize;
  uint8  ReservedPercent;

public:

  uint64  TotalBlocks;
  uint32  GroupCount;
  uint32  GroupDescPerBlock;
  uint32  GroupDescBlockCnt;
  uint32  BlockOfSuperBlock;
  uint32  BlocksOfLastGroup;
  uint32  RsvdGdtBlockOffset;
  uint32  InodeBlocksPerGroup;
};

#endif
