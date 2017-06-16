#include "BlockGroup.h"
#include "Tables.h"
#include "GlobalDef.h"
#include "BaseError.h"
#include "Inode.h"
#include <cassert>
#include <algorithm>

CBlockGroup::CBlockGroup(uint32 Index, TSuperBlock& Super, CExt2Params& Params, CBlockManager& BlockMan)
  : _GroupId(Index),
    _Super(Super),
    _Params(Params),
    _BlockMan(BlockMan)
{  
  uint32 lastGroup = Params.GroupCount - 1;

  StartBlock = Super.BlocksPerGroup * Index;
  _BlockCount = _GroupId == lastGroup ? 
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

  uint32 superBlockCnt = (1 + _Params.GroupDescBlockCnt + _Super.RsvdGdtBlocks) *
                          (_HaveSuperBlockBackup ? 1 : 0);

  RsvdGdtBlockOffset = StartBlock + _Params.RsvdGdtBlockOffset;
                          
  _BlockBmpBlocks = div_ceil(blockBmpSize, CExt2Params::BlockSize);
  _InodeBmpBlocks = div_ceil(inodeBmpSize, CExt2Params::BlockSize);
  _InodeTableBlocks = div_ceil(_Super.InodesPerGroup * CExt2Params::InodeSize, CExt2Params::BlockSize);

  _FsBlocks = superBlockCnt + _BlockBmpBlocks + _InodeBmpBlocks + _InodeTableBlocks; 
  
  Desc.BlockBmpBlock = StartBlock + superBlockCnt;
  
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

  _InodeBmp.resize(inodeBmpSize, 0x00);
  _BlockBmp.resize(blockBmpSize, 0x00);
}


std::vector<byte>& CBlockGroup::GetBlockBmp()
{
  return _BlockBmp;
}

std::vector<byte>& CBlockGroup::GetInodeBmp()
{
  return _InodeBmp;
}

/*
 *  Synchronous block bitmap from block manager and update descript table.
 */
void CBlockGroup::UpdateGroupInfo(CBlockManager& BlockMan)
{
  uint32 bmpOffset = StartBlock / 8;
  uint32 bmpLen = _Params.BlocksPerGroup / 8;

  vector<byte>& globBmp = BlockMan.GetBlockBmp();

  if (globBmp.size() <= bmpOffset) {
    throw CError(L"Incorrect Global Block BMP size");
  }

  _BlockBmp.resize(_Params.BlockSize, 0);  
  memcpy((byte*)&_BlockBmp[0], (byte*)&globBmp[bmpOffset], bmpLen);    

  Desc.DirectoriesCount = 0;
  Desc.FreeBlockCount = _GroupId == (_Params.GroupCount - 1) ? 
                          _Params.BlocksOfLastGroup : _Params.BlocksPerGroup;

  for (uint32 block = 0; block < _Params.BlocksPerGroup; ++block) {
    if (_BlockBmp[block / 8] & (0x01 << (block % 8))) {
      --Desc.FreeBlockCount; 
    }
  }
  inode_map::iterator it = _InodeMap.begin();

  while (it != _InodeMap.end()) {
    it->second->GetIndex();

    if (it->second->Type == LINUX_S_IFDIR)
      ++Desc.DirectoriesCount;
    ++it;
  }
}

void CBlockGroup::UpdateInodeTables()
{
  assert((_InodeMap.size() + Desc.FreeInodeCount) == _Super.InodesPerGroup);

  inode_map::iterator it = _InodeMap.begin();

  while (it != _InodeMap.end()) {
    it->second->UpdateInodeTable();
    ++it;
  }
}

/* Flush inode data into the inode blocks */
void CBlockGroup::FlushInodeTables(CBlockManager& BlockMan)
{
  uint32 inodeCnt = _InodeMap.size();
	uint32 flush_size = inodeCnt * _Super.InodeSize;
	uint32 flush_blocks = div_ceil(flush_size, _Params.BlockSize);
  uint32 mod_time_offset = offsetof(TInode, ModificationTime);
  uint32 struct_size = sizeof(TInode);
  bool   extend_struct = (_Params.BlockSize / 2) >= struct_size ? true : false;

  assert((inodeCnt + Desc.FreeInodeCount) == _Super.InodesPerGroup);
	
	if (flush_blocks == 0) {
  	return;
	}
  uint32 inodesPerBlock = _Params.BlockSize / _Super.InodeSize;
  uint32 gbOffset = Desc.InodeTableBlock;

  inode_map::iterator it = _InodeMap.begin();

  while (it != _InodeMap.end()) {
    CInode* inode = it->second;

    uint32 block = gbOffset + inode->GetIndex() / inodesPerBlock;
    uint32 offset = (inode->GetIndex() % inodesPerBlock) * _Super.InodeSize;

    Bulk<byte>* buffer = BlockMan.GetSingleBlockDataBuffer(block);

    if (buffer == NULL) {
      /* The buffer of the block has not yet been created */
      buffer = BlockMan.CreateSingleBlockDataBuffer(block);
      memset(buffer->Data(), 0x00, buffer->Size());
    }
    byte* ptr = buffer->Data() + offset;

    Bulk<byte> temp(_Super.InodeSize);
    memset(temp.Data(), 0x00, temp.Size());
    memcpy(temp.Data(), (byte*)(&inode->Inode), struct_size);

    if (extend_struct) {    
      uint32* pModTime1 = (uint32*)(temp.Data() + mod_time_offset);
      uint32* pModTime2 = (uint32*)(temp.Data() + struct_size + mod_time_offset);
      *pModTime2 = *pModTime1;
    }
    memcpy(ptr, temp.Data(), _Super.InodeSize);

    //TInode* pInode = (TInode*)ptr;
    //*pInode = inode->Inode;

    ++it;
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
	if (GroupIndex == 0) 
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
  if (!_BlockMan.OccupyBlock(StartBlock, _FsBlocks)) {
    throw CError(L"Failed to occupy the file system needed blocks in the block group");
  }
}

uint CBlockGroup::GetStartInodeIndex()
{
  return _Super.InodesPerGroup * _GroupId;
}

uint CBlockGroup::GetInodeCount()
{
  return _InodeMap.size();
}

bool CBlockGroup::IsInodeExists(uint32 Inode)
{
  _InodeMap.find(Inode) == _InodeMap.end() ? false : true;
}

/* Have free inode to be allocated */
bool CBlockGroup::HaveFreeInode()
{
  return Desc.FreeInodeCount == 0 ? false : true;
}

/* Have super block backup */
bool CBlockGroup::HaveSuperBlockBackup()
{
  return _HaveSuperBlockBackup;
}

/* Acquire inode from this group */
CInode* CBlockGroup::AllocateNewInode(uint32 Type)
{
  if (Desc.FreeInodeCount == 0) {
    return NULL;
  }

  for (uint32 i = 0; i < _Super.InodesPerGroup; ++i) {
    if (!inode_used(i)) {
      CInode* inode = CInode::Create(Type, _Params.BlockSize);
      uint32 index = _GroupId * _Super.InodesPerGroup + i;

      _InodeMap.insert(make_pair(index, inode));

      --Desc.FreeInodeCount;
      inode->SetIndex(index);
      inode->SetGroupID(_GroupId);
      set_inode_used(i);
      return inode;
    }    
  }
  return NULL;
}

/* Acquire specific inode number from this group */
bool CBlockGroup::OccupyInodeNumber(CInode* Inode, uint32 InodeNo)
{
  if (inode_used(InodeNo)) {
    //throw CError(L"The indicated inode has been occupied.");
    return false;
  }
  _InodeMap.insert(make_pair(InodeNo, Inode));
  --Desc.FreeInodeCount;
	Inode->SetIndex(InodeNo);
	Inode->SetGroupID(_GroupId);
  set_inode_used(InodeNo); 
  return true;
}

