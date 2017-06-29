//---------------------------------------------------------------------------
#include "Pch.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "JournalInodeReader.h"
#include "JournalSpWrapper.h"
#include "GlobalDef.h"
#include "fs_assert.h"
//---------------------------------------------------------------------------
CJournalInodeReader::CJournalInodeReader(TInode* InodeIn, uint16 BlockSize)
  : CInodeReader(InodeIn, BlockSize)  
{
}

CJournalInodeReader::~CJournalInodeReader()
{
}

void CJournalInodeReader::Read(CBlockManager& BlockMan, CUsbDrive* Drive)
{
  RebuildInodeBlocks(BlockMan, Drive);

  /* Just read a part of blocks to check */
  uint32 check_blocks = min_of(1024, GetTotalDataBlocks());
  ReadInodeData(BlockMan, Drive, check_blocks);
}

/*
 *  Parse journal super block from data and check contents.
 */
void CJournalInodeReader::CheckJournalSuperBlock()
{
  CJournalSpWrapper wrapper(_DataBuffer.Data());

  TJournalSuperBlock& journalSp = wrapper.GetStruct();

  fs_assert_eq(journalSp.Header.Signature, 0xc03b3998);
}


