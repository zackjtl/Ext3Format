//---------------------------------------------------------------------------
#ifndef ResizeInodeH
#define ResizeInodeH
//---------------------------------------------------------------------------
#include "Inode.h"
#include "BaseTypes.h"
#include "Ext2Params.h"
#include "Tables.h"

class CResizeInode : public CInode
{
public:
  CResizeInode(uint32 Block_Size);

  using CInode::WriteData;

  void WriteData(CBlockManager& BlockMan, TSuperBlock& Super, CExt2Params& Params);
  void UpdateInodeTable();

private:
  bool    _written;
  uint32  _middle_block_cnt;
};


#endif
