#ifndef BlockGroupH
#define BlockGroupH

#include "Tables.h"
#include "Ext2Params.h"
#include "BitArray.h"
#include "BlockManager.h"
#include "Inode.h"
#include <string>
#include <vector>

class CBlockGroup
{
public:
  CBlockGroup(uint32 Index, TSuperBlock& Super, CExt2Params& Params, CBlockManager& BlockMan);
  ~CBlockGroup();

  TGroupDesc& GetDescriptor() { return Desc; }

	void UpdateGroupInfo(CBlockManager& BlockMan);
	void UpdateInodeTables();
  
  void OccupyFileSystemBlocks();
  uint GetStartInodeIndex();  

  /* Acquire inode from this group */
  bool HaveFreeInode();
  bool HaveSuperBlockBackup();
  CInode* AllocateNewInode(uint8 Type);
  bool OccupyInodeNumber(CInode* Inode, uint32 InodeNumber);    

  std::vector<byte>& GetBlockBmp();
  std::vector<byte>& GetInodeBmp();

  void FlushInodeTables(CBlockManager& BlockMan);

public:
  TGroupDesc      Desc;

  uint32          StartBlock;

  /* The offset block of reserved gdt block */
  uint32          RsvdGdtBlockOffset;  

public:
  static bool bg_has_super(TSuperBlock& Super, uint32 GroupIndex);

private:
  
  /* Initial descriptor and bitmaps */
  void Initial();

private:
  uint32        _GroupId;  
  uint32        _BlockCount;
  TSuperBlock&  _Super;
  CExt2Params&  _Params;
  bool          _HaveSuperBlockBackup;
  CBlockManager& _BlockMan;  
  uint32        _FsBlocks;

  uint32        _BlockBmpBlocks;
  uint32        _InodeBmpBlocks;
  uint32        _InodeTableBlocks;

  std::vector<byte>     _BlockBmp;
  std::vector<byte>     _InodeBmp;
  std::vector<CInode*>  _InodeList;
};

static int test_root(uint32 a, uint32 b);


#endif
