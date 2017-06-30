//---------------------------------------------------------------------------
#include "Pch.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "FolderInodeReader.h"
#include "e3fs_tables.h"
#include "e3fs_assert.h"
//---------------------------------------------------------------------------
CFolderInodeReader::CFolderInodeReader(TInode* InodeIn, uint16 BlockSize)
  : CInodeReader(InodeIn, BlockSize)
{
  DotInode = EXT2_ROOT_INO - 1;
  TwoDotInode = EXT2_ROOT_INO - 1;
}

CFolderInodeReader::~CFolderInodeReader()
{

}

void CFolderInodeReader::AddEntry(ext2_dir_entry& Entry)
{
  std::string name = Entry.name;
  _NameList.insert(make_pair(name, Entry.inode));
}

/*
 *  For directory inodes, read all inode data to rebuild entries.
 */
void CFolderInodeReader::Read(CBlockManager& BlockMan, CUsbDrive* Drive)
{
  RebuildInodeBlocks(BlockMan, Drive);
  ReadInodeData(BlockMan, Drive);
}

