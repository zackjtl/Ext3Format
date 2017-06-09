#ifndef BlockGroupH
#define BlockGroupH

#include "Tables.h"
#include "Ext2Params.h"
#include "BitArray.h"
#include "BlockManager.h"
#include <string>
#include <vector>

class CBlockGroup
{
public:
  CBlockGroup(uint32 Index, TSuperBlock& Super, CExt2Params& Params, CBlockManager& BlockMan);
  ~CBlockGroup();

  TGroupDesc& GetDescriptor() { return Desc; }
  
  void OccupyFileSystemBlocks();

  uint GetStartInodeIndex();  

  /* Acquire inode from this group */
  bool HaveFreeInode();
  uint32 AllocateNewInode();

  void CreateRsvdGdtBlockData();

public:
  TGroupDesc      Desc;

public:
  static bool bg_has_super(TSuperBlock& Super, uint32 GroupIndex);

private:
  
  /* Initial descriptor and bitmaps */
  void Initial();

private:
  uint32        _Index;
  uint32        _StartBlock;
  uint32        _BlockCount;
  TSuperBlock&  _Super;
  CExt2Params&  _Params;
  bool          _HaveSuperBlockBackup;
  CBlockManager& _BlockMan;  
  uint32        _FsBlocks;

  uint32        _BlockBmpBlocks;
  uint32        _InodeBmpBlocks;
  uint32        _InodeTableBlocks;

  /* The offset block of reserved gdt block */
  uint32        _RsvdGdtBlockOffset;  

  std::vector<byte>   _InodeBmp;
};

static int test_root(uint32 a, uint32 b);


#endif
