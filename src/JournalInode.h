//---------------------------------------------------------------------------
#ifndef JournalInodeH
#define JournalInodeH
//---------------------------------------------------------------------------
#include "Inode.h"
#include "BlockManager.h"
#include "Ext2Params.h"
#include "e3fs_tables.h"

class CJournalInode : public CInode
{
public:
  CJournalInode(uint32 BlockSize, uint32 JournalBlocks);
  ~CJournalInode();

  void SetData(CBlockManager& BlockMan, TSuperBlock& Super, CExt2Params& Params);
  void InitJournalSuperBlock(TSuperBlock& Super, CExt2Params& Params);
  void UpdateInodeTable();

private:

  uint32              _JournalBlocks;
  TJournalSuperBlock  _JournalSp;

};

#endif
