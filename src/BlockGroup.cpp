#include "BlockGroup.h"
#include "Tables.h"
#include "GlobalDef.h"

/* 
 *  This constructor is used for a new created file system initiate
 */
CBlockGroup::CBlockGroup(uint32 StartBlock, uint32 BlockCount, CExt2Params& Ext2Params, bool HaveSuperBlockBackup)
  : _StartBlock(StartBlock),
    _BlockCount(BlockCount),
    _HaveSuperBlockBackup(HaveSuperBlockBackup)
{  
  InitialByExt2Params(Ext2Params)
}

~CBlockGroup()
{
}

/* 
 *  Initial descriptor and bitmaps
 */
void CBlockGroup::InitialByExt2Params(CExt2Params& Ext2Params)
{
  uint32 inodes = Ext2Params.GetInodesPerGroup();

  uint32 blockBmpSize = get_need_cnt(CExt2Params::BlocksPerGroup, 8);
  uint32 inodeBmpSize = get_need_cnt(inodes, 8);

  uint32 superBlockCnt = _HaveSuperBlockBackup ? 1 : 0;
  uint32 descBlockCnt = _HaveSuperBlockBackup ? Ext2Params.GetDescBlockCount() : 0;
  uint32 rsvdGdtBlockCnt = _HaveSuperBlockBackup ? Ext2Params.GetGdtReservedBlocks() : 0;
  uint32 blockBmpBlocks = get_need_cnt(blockBmpSize, CExt2Params::BlockSize);
  uint32 inodeBmpBlocks = get_need_cnt(inodeBmpSize, CExt2Params::BlockSize);
  uint32 inodeBlocks = get_need_cnt(Ext2Params.GetInodesPerGroup() * CExt2Params::InodeSize, CExt2Params::BlockSize);

  uint32 usedBlocks = superBlockCnt + descBlockCnt + rsvdGdtBlockCnt +
                      blockBmpBlocks + inodeBmpBlocks + inodeBlocks; 
  
  _Desc.BlockBmpBlock = _StartBlock + superBlockCnt + descBlockCnt + rsvdGdtBlockCnt;
  _Desc.InodeBmpBlock = _Desc.BlockBmpBlock + blockBmpBlocks;
  _Desc.FreeBlockCount = CExt2Params::BlocksPerGroup - usedBlocks;
  _Desc.FreeInodeCount = inodes;
  _Desc.DirectoriesCount = 0;
  _Desc.Flags = 0;
  _Desc.SnapshotExcludeBmp = 0;
  _Desc.BlockBmpChecksum = 0;
  _Desc.InodeBmpChecksum = 0;
  _Desc.NeverUsedInodeCnt = 0;
  _Desc.GroupDescChecksum = 0;

  if (blockBmpSize) {
    _BlockBmp.Resize(CExt2Params::BlocksPerGroup);
    _BlockBmp.Reset();

    for (int i = 0; i < usedBlocks; ++i) {
      _BlockBmp.Set(i, true);
    }
  }
  if (inodeBmpSize) {
    _InodeBmp.Resize(inodes);
    _InodeBmp.Reset();
  }
  
}

/* Acquire inode from this group */
void CBlockGroup::AllocateInode()
{
  // TODO:
}

/* Acquire blocks from this group */
void CBlockGroup::AllocateBlock(uint32 Count)
{
  // TODO:
}
