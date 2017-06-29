//---------------------------------------------------------------------------
#pragma hdrstop
#include "JournalInode.h"
#include "TypeConv.h"
#include "GlobalDef.h"
#include "JournalSpWrapper.h"
#include "my_uuid.h"
#include <cstring>

//---------------------------------------------------------------------------
CJournalInode::CJournalInode(uint32 BlockSize)
  : CInode(LINUX_S_IFREG, BlockSize)
{
  Permissions = 600;
  _JournalSize = (uint32)128 << 20; // 128MB
}

CJournalInode::~CJournalInode()
{
}

void CJournalInode::SetData(CBlockManager& BlockMan, TSuperBlock& Super, CExt2Params& Params)
{
  Bulk<byte>  buffer(_JournalSize);

  byte* ptr = buffer.Data();
  
  InitJournalSuperBlock(Super, Params);

  CJournalSpWrapper wrapper(_JournalSp);

  wrapper.ToBuffer(buffer);

  ////memcpy(ptr, (byte*)&_JournalSp, sizeof(TJournalSuperBlock));

  CInode::SetData(BlockMan, buffer.Data(), buffer.Size());
}

void CJournalInode::InitJournalSuperBlock(TSuperBlock& Super, CExt2Params& Params)
{
  _JournalSp.Header.Signature = 0xc03b3998;
  _JournalSp.Header.BlockType = 4;
  _JournalSp.Header.SequenceNumber = 0;
  _JournalSp.BlockSize = (uint32)Params.BlockSize;
  _JournalSp.BlockCount = _JournalSize / _JournalSp.BlockSize;
  _JournalSp.StartBlock = 1;
  _JournalSp.SequenceOf1stTrans = 1;
  _JournalSp.BlockOf1stTrans = 0;
  _JournalSp.Error = 0;
  _JournalSp.CompFeatureFlags = 0;
  _JournalSp.IncompFeatureFlags = 0;
  _JournalSp.ROCompFeatureFlags = 0;

  gen_uuid_v4(_JournalSp.UUID, 16);
  ////memcpy(_JournalSp.UUID, Super.JournalUUID, UUID_SIZE);

  _JournalSp.Users = 1;
  _JournalSp.BlocksOfDynamicSBCopy = 0;
  _JournalSp.MaxTransaction = 0;
  _JournalSp.MaxTransactionBlocks = 0;
  _JournalSp.ChecksumType = 0;

  memset(_JournalSp.Padding2, 0, sizeof(_JournalSp.Padding2));
  memset((byte*)&_JournalSp.Padding, 0, sizeof(_JournalSp.Padding));

  _JournalSp.Checksum = 0;
  memset(_JournalSp.User_ID, 0, sizeof(_JournalSp.User_ID));
  
}

void CJournalInode::UpdateInodeTable()
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
  Inode.HardLinkCnt = Type == LINUX_S_IFREG ? 1 : 0;
  Inode.OS_Dep1 = 0;
  Inode.FileVersion = 0;
  Inode.FragAddress = 0;
  Inode.FragNumber = 0;
  Inode.FragSize = 0;
  Inode.Padding = 0;
  memset((byte*)&Inode.Rsvd[0], 0, sizeof(Inode.Rsvd));  
}


