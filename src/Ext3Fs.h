#ifndef Ext3FsH
#define Ext3FsH

#include "Tables.h"
#include "Types.h"
#include <vector>

using namespace std;

class CExt3Fs
{
public:
  CExt3Fs(uint32 TotalSectors);
  ~CExt3Fs();


private:
  uint32  _TotalSectors;
  uint32  _TotalBlocks;
  uint32  _BlockGroupCount;

  vector<CBlockGroup> _BlockGroups;

  CExt2Params         _Params;
};

#endif
