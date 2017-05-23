#ifndef Ext2ParamsH
#define Ext2ParamsH

#include "Types.h"
#include <vector.h>
#include "SuperBlock.h"
#include "BlockGroup.h"

using namespace std;

class CExt2Params
{
public:
  CExt2Params(uint32 TotalSectors);
  ~CExt2Params();

  uint32  GetTotalBlocks()        { return _TotalBlocks;        }
  uint32  GetGroupCount()         { return _GroupCount;         }   
  uint32  GetInodesPerGroup()     { return _InodesPerGroup;     } 
  uint32  GetTotalInodes()        { return _TotalInodes;        }
  uint32  GetGdtReservedBlocks()  { return _GdtReservedBlocks;  }
  uint32  GetDescBlockCount()     { return _DescBlockCount;     }

  void InitSuperBlock();
  void InitBlockGroups();
  
public:
  static uint32 BlockSize;
  static uint32 InodeSize;
  static uint32 BlocksPerGroup;
  static uint32 InodeRatio;
  static uint32 TimeOfResize;

private:

  //   
  uint32  _TotalSectors;
  uint32  _TotalBlocks;
  uint32  _GroupCount;  

  uint32  _InodesPerGroup;
  uint32  _TotalInodes;
  uint32  _GdtReservedBlocks;
  uint32  _DescBlockCount;
  
};

#endif
