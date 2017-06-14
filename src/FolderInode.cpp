//---------------------------------------------------------------------------
#pragma hdrstop
#include "FolderInode.h"
//---------------------------------------------------------------------------
CFolderInode::CFolderInode(uint32 BlockSize)
  : CInode(EXT2_FT_DIR, BlockSize),
    _Initialized(false)
{
	_DotInode = 0;
	_TwoDotInode = 0;
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

	_DotInode = DotInode;
	_TwoDotInode = TwoDotInode;

	ext2_dir_entry 	dot_ent;
	MakeEntry(dot_ent, DotInode, EXT2_FT_DIR, " .", Super);
	this->WriteData(BlockMan, (byte*)&dot_ent, dot_ent.rec_len);

	ext2_dir_entry 	two_dot_ent;
	MakeEntry(two_dot_ent, TwoDotInode, EXT2_FT_DIR, "..", Super);
	this->WriteData(BlockMan, (byte*)&two_dot_ent, two_dot_ent.rec_len);

	_Initialized = true;
}

/*
 * 	Attach a sub folder or file inode on this inode
 */
void CFolderInode::Attach(CInode* Inode, CBlockManager& BlockMan, TSuperBlock& Super)
{
	if (Inode->Mode == EXT2_FT_DIR) {
		CFolderInode* folder = (CFolderInode*)Inode;
		folder->MkDir(this->GetIndex(), _DotInode, BlockMan, Super);
	}
	ext2_dir_entry  entry;
	MakeEntry(entry, Inode->GetIndex(), Inode->Mode, Inode->GetName().c_str(), Super);

	_NameList.insert(make_pair(Inode->GetName(), Inode->GetIndex()));
	this->WriteData(BlockMan, (byte*)&entry, entry.rec_len);
}

/*
 * 	Specific the entry's name if the argument Name is a non-null string,
 *  or use the inode's original name.
 */
void CFolderInode::MakeEntry(ext2_dir_entry& Entry, uint32 InodeNo, uint16 Mode,
															const char* Name, TSuperBlock& Super)
{
	uint16 nameLen = Name == NULL ? 0 : strlen((char*)Name);

	/* The inode index in entry table is started from 1 */
	Entry.inode = InodeNo + 1;
	Entry.rec_len = 8 + nameLen + 2;
	Entry.name_len = nameLen;

	if (Super.IncompFeatureFlags & EXT2_FEATURE_INCOMPAT_FILETYPE) {
		Entry.name_len |= (Mode & 0x7) << 8;
	}
	memset(Entry.name, 0, sizeof((char*)Entry.name));
	strcpy(Entry.name, (char*)Name);
}
                              
void CFolderInode::UpdateInodeTable()
{
  Inode.Mode = (Mode << 12) + Permissions;
  Inode.Uid = 0;
  Inode.SizeInBytesLo = (uint32)_Size;
  Inode.SizeInBytesHi = _Size >> 32;
  time((long*)&Inode.AccessTime);
  Inode.InodeChangeTime = Inode.AccessTime;
  Inode.ModificationTime = Inode.AccessTime;
  Inode.DeleteTime = 0;
  Inode.GroupId = _GroupID;
  Inode.HardLinkCnt = 0;
  Inode.SectorCount = _Size / 512;
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