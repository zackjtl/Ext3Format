//---------------------------------------------------------------------------
#pragma hdrstop
#include "FileInode.h"
//---------------------------------------------------------------------------
CFileInode::CFileInode(uint32 BlockSize)
  : CInode(EXT2_FT_REG_FILE, BlockSize)
{
}

CFileInode::~CFileInode()
{  
}

