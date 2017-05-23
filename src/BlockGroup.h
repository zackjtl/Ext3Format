#ifndef BlockGroupH
#define BlockGroupH

#include "Tables.h"
#include "Ext2Params.h"
#include "BitArray.h"
#include <string>

using namespace std;

class CBlockGroup
{
public:
  CBlockGroup(uint32 StartBlock, uint32 BlockCount, CExt2Params& Ext2Params, bool HaveSuperBlockBackup);
  ~CBlockGroup();

  TGroupDesc& GetDescriptor() { return _Desc; }
  CBitArray& GetBlockBmp()    { return _BlockBmp; }
  CBitArray& GetInodeBmp()    { return _InodeBmp; }

  /* Acquire inode from this group */
  void AllocateInode();

  /* Acquire blocks from this group */
  void AllocateBlock(uint32 Count);

private:
  
  /* Initial descriptor and bitmaps */
  void InitialByExt2Params(CExt2Params& Ext2Params);

private:
  uint32        _StartBlock;
  uint32        _BlockCount;
  TSuperBlock   _SuperBlock;
  bool          _HaveSuperBlockBackup;

  TGroupDesc    _Desc;    

  CBitArray     _BlockBmp;
  CBitArray     _InodeBmp;
  
};

#endif
