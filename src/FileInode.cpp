//---------------------------------------------------------------------------
#pragma hdrstop
#include "FileInode.h"
//---------------------------------------------------------------------------
CFileInode::CFileInode(uint32 BlockSize)
  : CInode(LINUX_S_IFREG, BlockSize)
{
}

CFileInode::~CFileInode()
{  
}

