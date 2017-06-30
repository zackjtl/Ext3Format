//---------------------------------------------------------------------------
#pragma hdrstop
#include "FolderInode.h"
#include "TypeConv.h"
#include "e3fs_def.h"
#include <time.h>
#include <vector>
#include <cassert>
#include <cstring>

using namespace std;
//---------------------------------------------------------------------------
CFolderInode::CFolderInode(uint16 BlockSize)
  : CInode(LINUX_S_IFDIR, BlockSize),
    _Initialized(false),
    _LastEntryOffset(0),
    _LastEntryBlock(0)
{
	DotInode = EXT2_ROOT_INO - 1;
	TwoDotInode = EXT2_ROOT_INO - 1;
}

CFolderInode::~CFolderInode()
{  
}


/*
 *  Initialize a new directory with write its dot and 2-dot entries
 */
void CFolderInode::MkDir(uint32 DotInode, uint32 TwoDotInode, CBlockManager& BlockMan, TSuperBlock& Super)
{
	if (_Initialized)
		return;

	DotInode = DotInode;
	TwoDotInode = TwoDotInode;

  if (alloc_blocks(BlockMan, 1) != 0) {
    throw CError(L"Allocate block for directory failed");
  }
  uint32 lastBlock = GetLastRealBlock();
  uint32 real_len;
  Bulk<byte>* buffer = BlockMan.CreateSingleBlockDataBuffer(lastBlock);
  byte* ptr = buffer->Data();

  memset(buffer->Data(), 0x00, buffer->Size());

	ext2_dir_entry 	dotEntry;
	MakeEntry(dotEntry, DotInode, EXT2_FT_DIR, ".", Super);
  memcpy(ptr, (byte*)&dotEntry, dotEntry.rec_len);

  _NameList.insert(make_pair(".", DotInode));

  ptr += dotEntry.rec_len;

	ext2_dir_entry 	twoDotEntry;
	MakeEntry(twoDotEntry, TwoDotInode, EXT2_FT_DIR, "..", Super);

  _NameList.insert(make_pair("..", TwoDotInode));

  _LastEntryOffset = dotEntry.rec_len;
  real_len = twoDotEntry.rec_len;
  twoDotEntry.rec_len = _BlockSize - dotEntry.rec_len;

  memcpy(ptr, (byte*)&twoDotEntry, real_len);

  _Size = _BlockSize;
  _Position = _BlockSize;
	_Initialized = true;
}

#define filetype_en() (Super.IncompFeatureFlags & EXT2_FEATURE_INCOMPAT_FILETYPE)

/* To calculate the real entry size instead the rec_len variable. */
#define real_entry_len(x) (10 + (x->name_len & (filetype_en() ? 0x0f : 0xff )))

/* Convert inode type to the file type of the entry */
#define get_file_type(x) (x == LINUX_S_IFDIR ? EXT2_FT_DIR : EXT2_FT_REG_FILE)

/*
 * 	Attach a sub folder or file inode on this inode
 */
void CFolderInode::Attach(CInode* Inode, CBlockManager& BlockMan,
                          TSuperBlock& Super)
{
	if (Inode->Type == LINUX_S_IFDIR) {
		CFolderInode* folder = (CFolderInode*)Inode;
		folder->MkDir(Inode->GetIndex(), _Index, BlockMan, Super);
	}
	ext2_dir_entry  entry;
  uint8 file_type = get_file_type(Inode->Type);
	MakeEntry(entry, Inode->GetIndex(), file_type, Inode->GetName().c_str(), Super);

	_NameList.insert(make_pair(Inode->GetName(), Inode->GetIndex()));
	////this->SetData(BlockMan, (byte*)&entry, entry.rec_len);

  AddEntry(entry, BlockMan, Super);
}

/*
 * 	Attach a sub folder or file inode on this inode
 */
void CFolderInode::AddEntry(ext2_dir_entry& Entry, CBlockManager& BlockMan,
                            TSuperBlock& Super)
{
  uint32 lastBlock = GetLastRealBlock();
  uint32 real_rec_len;
  uint32 remain_len;
  byte* ptr;
  ext2_dir_entry* pEntry;
  
  Bulk<byte>* buffer = BlockMan.GetSingleBlockDataBuffer(lastBlock);

  if (_Size == 0 || buffer == NULL) {
    goto NEW_ALLOC_BLOCK;
  }

  ptr = buffer->Data() + _LastEntryOffset;
  pEntry = (ext2_dir_entry*)ptr;

  real_rec_len = real_entry_len(pEntry);

  /* replace the record length by the realize  */
  pEntry->rec_len = real_rec_len;
  ptr += real_rec_len;  

  if ((_LastEntryOffset + real_rec_len + Entry.rec_len) > _BlockSize) {
    goto NEW_ALLOC_BLOCK;
  }
  /* replace the last entry's record length by the remain length */
  remain_len = _BlockSize - (_LastEntryOffset + real_rec_len);
  real_rec_len = Entry.rec_len;
  Entry.rec_len = remain_len;   
  
  memcpy(ptr, (byte*)&Entry, real_rec_len);
  return;

NEW_ALLOC_BLOCK:

  if (alloc_blocks(BlockMan, 1) != 0) {
    throw CError(L"Allocate block for directory failed");
  }
  lastBlock = GetLastRealBlock();

  buffer = BlockMan.CreateSingleBlockDataBuffer(lastBlock);
  real_rec_len = Entry.rec_len;
  Entry.rec_len = _BlockSize;

  memset(buffer->Data(), 0x00, buffer->Size());
  memcpy(buffer->Data(), (byte*)&Entry, real_rec_len);    
  _Position += _BlockSize;
  _Size += _BlockSize;
  _LastEntryOffset = 0;     
  return;    
}

/*
 * 	Specific the entry's name if the argument Name is a non-null string,
 *  or use the inode's original name.
 */
void CFolderInode::MakeEntry(ext2_dir_entry& Entry, uint32 InodeNo, uint8 FileType,
															const char* Name, TSuperBlock& Super)
{
	uint16 nameLen = Name == NULL ? 0 : strlen((char*)Name);

	/* The inode index in entry table is started from 1 */
	Entry.inode = InodeNo + 1;
	Entry.rec_len = 8 + nameLen + 2;
	Entry.name_len = nameLen;

  if (Entry.rec_len < 12) {
    Entry.rec_len = 12;
  }
	if (Super.IncompFeatureFlags & EXT2_FEATURE_INCOMPAT_FILETYPE) {
		Entry.name_len |= (FileType & 0x7) << 8;
	}
	memset(Entry.name, 0, sizeof((char*)Entry.name));
	strcpy(Entry.name, (char*)Name);
}
                              
void CFolderInode::UpdateInodeTable()
{
  Inode.Mode = OctToDec(Type + Permissions);
  Inode.Uid = 0;
  Inode.SizeInBytesLo = (uint32)_Size;
  Inode.SizeInBytesHi = _Size >> 32;
  Inode.AccessTime = GetPosixTime();
  Inode.InodeChangeTime = Inode.AccessTime;
  Inode.ModificationTime = Inode.AccessTime;
  Inode.DeleteTime = 0;
  Inode.GroupId = _GroupID;
  Inode.SectorCount = CalculateSectorCount(_Size);
  Inode.FileFlags = 0;
  Inode.HardLinkCnt = _NameList.size();  /* For directory, this is the sub item number */
  Inode.OS_Dep1 = 0;
  Inode.FileVersion = 0;
  Inode.FragAddress = 0;
  Inode.FragNumber = 0;
  Inode.FragSize = 0;
  Inode.Padding = 0;

  memset((byte*)&Inode.Rsvd[0], 0, sizeof(Inode.Rsvd));
}
