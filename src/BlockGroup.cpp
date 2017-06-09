#include "BlockGroup.h"
#include "Tables.h"
#include "GlobalDef.h"
#include "BaseError.h"
#include "Inode.h"

CBlockGroup::CBlockGroup(uint32 Index, TSuperBlock& Super, CExt2Params& Params, CBlockManager& BlockMan)
  : _Index(Index),
    _Super(Super),
    _Params(Params),
    _BlockMan(BlockMan)
{  
  uint32 lastGroup = Params.GroupCount - 1;

  _StartBlock = Super.BlocksPerGroup * Index;
  _BlockCount = _Index == lastGroup ? 
                 Super.BlocksPerGroup : Params.BlocksOfLastGroup;

  _HaveSuperBlockBackup = bg_has_super(Super, Index);  

  Initial();
}

CBlockGroup::~CBlockGroup()
{
}

#define inode_used(x) (_InodeBmp[x / 8] & (0x01 << (x % 8)))
#define set_inode_used(x) {_InodeBmp[x / 8] |= (0x01 << (x % 8));}
#define unset_inode_used(x) {_InodeBmp[x / 8] &= ~(0x01 << (x % 8));}

/* 
 *  Initial descriptor and bitmaps
 */
void CBlockGroup::Initial()
{
  uint32 inodes = _Super.InodesPerGroup;

  uint32 blockBmpSize = div_ceil(_Super.BlocksPerGroup, 8);
  uint32 inodeBmpSize = div_ceil(inodes, 8);

  uint32 blockSize = (1 << _Super.BlockSizeFlag) * 1024;

  uint32 superBlockCnt = (1 + _Params.GroupDescBlockCnt + _Super.ReservedBlockCnt) * 
                          (_HaveSuperBlockBackup ? 1 : 0);

  _RsvdGdtBlockOffset = 1 + _Params.GroupDescBlockCnt; 
                          
  _BlockBmpBlocks = div_ceil(blockBmpSize, CExt2Params::BlockSize);
  _InodeBmpBlocks = div_ceil(inodeBmpSize, CExt2Params::BlockSize);
  _InodeTableBlocks = div_ceil(_Super.InodesPerGroup * CExt2Params::InodeSize, CExt2Params::BlockSize);

  _FsBlocks = superBlockCnt + _BlockBmpBlocks + _InodeBmpBlocks + _InodeTableBlocks; 
  
  Desc.BlockBmpBlock = _StartBlock + superBlockCnt;
  
  /* The bmp blocks was constrained in one block ? */
  Desc.InodeBmpBlock = Desc.BlockBmpBlock + _BlockBmpBlocks;   
  Desc.InodeTableBlock = Desc.InodeBmpBlock + _InodeBmpBlocks;
  Desc.FreeBlockCount = CExt2Params::BlocksPerGroup - _FsBlocks;
  Desc.FreeInodeCount = inodes;
  Desc.DirectoriesCount = 0;
  Desc.Flags = 0;
  Desc.SnapshotExcludeBmp = 0;
  Desc.BlockBmpChecksum = 0;
  Desc.InodeBmpChecksum = 0;
  Desc.NeverUsedInodeCnt = 0;
  Desc.GroupDescChecksum = 0;

  if (inodeBmpSize) {
    _InodeBmp.resize(inodeBmpSize, 0xFF);
  }
  
}

int test_root(uint32 a, uint32 b)
{
	while (1) {
		if (a < b)
			return 0;
		if (a == b)
			return 1;
		if (a % b) 
			return 0;
		a = a / b;
	}
}

bool CBlockGroup::bg_has_super(TSuperBlock& Super, uint32 GroupIndex) 
{
	if (GroupIndex) 
		return true;
		
	if (Super.CompFeatureFlags & EXT4_FEATURE_COMPAT_SPARSE_SUPER2) {
		/* In e2fsprogs, there is the condition determined by the 
		 		super->s_backup_bgs exists, but that is for ext4 so that 
		 		not appears here. */
		;
	}
	if ((GroupIndex <= 1) || !(Super.ROCompFeatureFlags) & 
															EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER)
		return true;
	
	if (!(GroupIndex & 1)) 
		return false;
	
	if (test_root(GroupIndex, 3) || test_root(GroupIndex, 5) || test_root(GroupIndex, 7))
		return true;
	
	return false;
}

/*
 *  Occupy the file system required blocks such as super block, gdt reserved block, 
 *   group dexcriptor blocks, block bitmap, inode bitmap blocks and inode blocks
 *   from the global block manager.
 */
void CBlockGroup::OccupyFileSystemBlocks()
{ 
  if (!_BlockMan.OccupyBlock(_StartBlock, _FsBlocks)) {
    throw CError(L"Failed to occupy the file system needed blocks in the block group");
  }
}

uint CBlockGroup::GetStartInodeIndex()
{
  return _Super.InodesPerGroup * _Index;
}

/* Have free inode to be allocated */
bool CBlockGroup::HaveFreeInode()
{
  return Desc.FreeInodeCount == 0 ? false : true;
}
/*
 *  Setup the Reserved GDT Block data.
 */

void CBlockGroup::CreateRsvdGdtBlockData()
{
  _RsvdGdtBlockOffset;

  Bulk<byte>  buffer(_Super.ReservedBlockCnt * _Params.BlockSize);
  memset(buffer, 0, buffer.Size());  

  if (_Index == 0) {
    /*  
     *  Only the gdt block of 1st group have the address table of the others 
     *  gdt blocks.
     */
    byte* ptr = NULL;    
    
    for (uint32 index = 0; index < _Super.ReservedBlockCnt; ++index) {    
      ptr = buffer.Data() + index * _Params.BlockSize;
      uint32* ptr32 = (uint32*)ptr;

      for (int gp = 1; gp < _Params.GroupCount; ++gp) {
        if (bg_has_super(_Super, gp)) {
          *ptr32 = gp * _Super.BlocksPerGroup + (1 +  _Params.GroupDescBlockCnt);
          ++ptr32;
        }
      }
    }
  }

  _BlockMan.SetBlockData(_RsvdGdtBlockOffset, buffer.Data(), buffer.Size());
}

/* Acquire inode from this group */
uint32 CBlockGroup::AllocateNewInode()
{
  if (Desc.FreeInodeCount == 0) {
    return CInode::invalid_inode;
  }

  for (uint32 i = 0; i < _Super.InodesPerGroup; ++i) {
    if (!inode_used(i)) {
      --Desc.FreeInodeCount;
      return i;
    }    
  }
  return CInode::invalid_inode;
}
