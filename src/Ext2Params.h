#ifndef Ext2ParamsH
#define Ext2ParamsH

#include "Types.h"
#include "Tables.h"
#include <vector>

using namespace std;

class CExt2Params
{
public:
  CExt2Params(uint64 TotalSectors);
  ~CExt2Params();

public:
  static uint32 BlockSize;
  static uint32 InodeSize;
  static uint32 BlocksPerGroup;
  static uint32 InodeRatio;
  static uint32 TimeOfResize;

public:

  uint64  TotalBlocks;
  uint32  GroupCount;  
  uint32  GroupDescPerBlock;
  uint32  GroupDescBlockCnt;
  uint32  BlocksOfLastGroup;
};

#endif
