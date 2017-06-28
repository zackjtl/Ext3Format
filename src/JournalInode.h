//---------------------------------------------------------------------------
#ifndef JournalInodeH
#define JournalInodeH
//---------------------------------------------------------------------------
#include "Inode.h"
#include "BlockManager.h"
#include "Ext2Params.h"
#include "Tables.h"

class CJournalInode : public CInode
{
public:
  CJournalInode(uint32 BlockSize);
  ~CJournalInode();

  void SetData(CBlockManager& BlockMan, TSuperBlock& Super, CExt2Params& Params);
  void InitJournalSuperBlock(TSuperBlock& Super, CExt2Params& Params);
  void UpdateInodeTable();

private:

  uint32              _JournalSize;
  TJournalSuperBlock  _JournalSp;

};

#endif
