//---------------------------------------------------------------------------
#ifndef ResizeInodeH
#define ResizeInodeH
//---------------------------------------------------------------------------
#include "Inode.h"
#include "BaseTypes.h"
#include "Ext2Params.h"
#include "e3fs_tables.h"

class CResizeInode : public CInode
{
public:
  CResizeInode(uint16 BlockSize);

  using CInode::SetData;

  void SetData(CBlockManager& BlockMan, TSuperBlock& Super, CExt2Params& Params);
  void UpdateInodeTable();

private:
  bool    _written;
  uint32  _middle_block_cnt;
};


#endif
