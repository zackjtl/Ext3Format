//---------------------------------------------------------------------------
#pragma hdrstop
#include "Ext3fsck.h"
#include "e3fs_def.h"
#include "e3fs_assert.h"
#include "InodeReader.h"
#include "FolderInodeReader.h"
#include "JournalInodeReader.h"
//---------------------------------------------------------------------------
CExt3Fsck::CExt3Fsck(CUsbDrive& Drive)
  : _Drive(Drive)
{
  _BlockBmpLoaded = false;
}
//---------------------------------------------------------------------------
CExt3Fsck::~CExt3Fsck()
{

}
//---------------------------------------------------------------------------
void CExt3Fsck::LoadFs()
{
  ReadSuperBlock();
  ReadAndInitBlockGroups();

  ReadBlockBmp();
  ReadInodeBmp();
  RebuildRootDir();

  if (Super.CompFeatureFlags & EXT3_FEATURE_COMPAT_HAS_JOURNAL) {
    RebuildJournalInode();
  } 
}
//---------------------------------------------------------------------------
void CExt3Fsck::ReadSuperBlock()
{
  uint32 sbSectors = div_ceil(sizeof(TSuperBlock), 512);
  uint32 totalSectors = _Drive.ReadCapacity();

  uint32 sbStartSector = Params.MBRSize / 512;

  Bulk<byte>  buffer(sbSectors * 512);

  _Drive.ReadSector(sbStartSector, sbSectors, buffer.Data());

  memcpy((byte*)&Super, buffer.Data(), sizeof(TSuperBlock));

  Params.BlockSize = (1 << Super.BlockSizeFlag) * 1024;

  uint32 sectorsPerBlock = Params.BlockSize / 512;

  uint64 totalBlocksExp = totalSectors / sectorsPerBlock;
  uint64 totalBlocksReal = ((uint64)Super.BlockCountHi << 32) +  Super.BlockCnt;

  fs_assert_eq(Super.Magic[0], 0x53);
  fs_assert_eq(Super.Magic[1], 0xEF);
  fs_assert_eq(totalBlocksExp, totalBlocksReal);
  
  Params.TotalBlocks = totalBlocksReal;
  Params.BlocksPerGroup = Super.BlocksPerGroup;
  Params.GroupCount = div_ceil(totalBlocksReal, Super.BlocksPerGroup);
  Params.InodeSize = Super.InodeSize;

  uint32 orgInodeCnt = Super.InodeCnt;
  uint32 orgInodesPerGp = Super.InodesPerGroup;

  CalcInodeNumber();

  fs_assert_eq(orgInodeCnt, Super.InodeCnt);
  fs_assert_eq(orgInodesPerGp, Super.InodesPerGroup);

  Params.CompleteParameters();
  Params.BlockOfSuperBlock = sbStartSector / sectorsPerBlock;

  CreateBlockManager();

}
//---------------------------------------------------------------------------
void CExt3Fsck::ReadAndInitBlockGroups()
{
  uint32 gpDescBlock = Params.BlockOfSuperBlock + 1;
  ////BlockMan->OccupyBlock(gpDescBlock, Params.GroupDescBlockCnt);

  Bulk<byte> buffer(Params.GroupDescBlockCnt * Params.BlockSize);

  ReadBlocksToBuffer(gpDescBlock, Params.GroupDescBlockCnt, buffer);

  TGroupDesc* pDesc = (TGroupDesc*)buffer.Data();
                    
  for (uint32 i = 0; i < Params.GroupCount; ++i) {
    CBlockGroup* ptr = new CBlockGroup(i, Super, Params, *BlockMan.get());   
  
    ptr->InitDescFromData(pDesc[i]);
    ////ptr->OccupyFileSystemBlocks();
    BlockGroups.push_back(ptr);
  }                  
  ////BlockMan->SetBlockData(gpDescBlock, buffer.Data(), buffer.Size());
}
//---------------------------------------------------------------------------
/*
 *  Read block bmp from media to set the bmp in the block groups and the block manager.
 */
void CExt3Fsck::ReadBlockBmp()
{ 
  uint32 bmpLen = Params.BlocksPerGroup / 8;
  uint32 blockBmpBlocks = div_ceil(bmpLen, Params.BlockSize);

	for (uint32 i = 0; i < Params.GroupCount; ++i) {
    CBlockGroup* gp = BlockGroups[i];
    uint32 bmpBlockStart = gp->Desc.BlockBmpBlock;
    uint32 bmpBlockEnd = bmpBlockStart + blockBmpBlocks;    
    uint32 remainBlocks = gp->GetBlockCount();
    uint32 blockPos = 0;

    for (uint32 block = bmpBlockStart; block < bmpBlockEnd; ++block) {
      Bulk<byte>* buffer = BlockMan->CreateSingleBlockDataBuffer(block);
      uint32 set_blk_cnt = min_of(remainBlocks, Params.BlockSize * 8);

      ReadBlocksToBuffer(block, 1, *buffer);

      gp->BulkSetBlockBmp(*buffer, blockPos, set_blk_cnt);      
      BlockMan->SetAreaBlockBmp(*buffer, gp->StartBlock + blockPos, set_blk_cnt);

      blockPos += set_blk_cnt;
      remainBlocks -= set_blk_cnt;
    }
    gp->ValidateFreeBlockCount();
  }
  _BlockBmpLoaded = true;
}
//---------------------------------------------------------------------------
/*
 *  Read inode bmp for global scope.
 */
void CExt3Fsck::ReadInodeBmp()
{ 
  uint32 inodeBmpBlocks = div_ceil((Super.InodesPerGroup / 8), Params.BlockSize);

  for (uint32 i = 0; i < Params.GroupCount; ++i) {
    CBlockGroup* gp = BlockGroups[i];
    uint32 bmpBlockStart = gp->Desc.InodeBmpBlock;
    uint32 bmpBlockEnd = bmpBlockStart + inodeBmpBlocks;
    uint32 remainInodes = Super.InodesPerGroup;
    uint32 inodePos = 0;

    for (uint32 block = bmpBlockStart; block < bmpBlockEnd; ++block) {
      Bulk<byte>* buffer =  BlockMan->CreateSingleBlockDataBuffer(block);
      uint32 set_inode_cnt = min_of(remainInodes, Params.BlockSize * 8);

      ReadBlocksToBuffer(block, 1, *buffer);

      gp->BulkSetInodeBmp(*buffer, inodePos, set_inode_cnt);

      inodePos += set_inode_cnt;
      remainInodes -= set_inode_cnt;
    }    
    gp->ValidateFreeInodeCount();
  }
}
//---------------------------------------------------------------------------
void CExt3Fsck::RebuildRootDir()
{
  TInode inode;

  GetOrReadInodeTable(EXT2_ROOT_INO - 1, inode);

  unique_ptr<CInodeReader>  inodeReader(CInodeReader::Create(
                                        &inode, Params.BlockSize));

  BlockGroups[0]->AddInodeWithSpecificNumber((CInode*)inodeReader.get(),
                                             EXT2_ROOT_INO - 1);

  if (!inodeReader->IsFolderInode()) {
    throw CError(L"Unexpected inode type of the root directory inode");
  }
  CFolderInodeReader* folder = (CFolderInodeReader*)inodeReader.get();

  folder->SetIndex(EXT2_ROOT_INO - 1);
  folder->Read(*BlockMan.get(), &_Drive);
  RebuildFolder(folder);
}
//---------------------------------------------------------------------------
void CExt3Fsck::RebuildFolder(CFolderInodeReader* Dir)
{

  Bulk<byte>& dataBuffer = Dir->GetDataBuffer();

  uint32 end = dataBuffer.Size();
  byte* ptr = dataBuffer.Data();
  uint32 offset = 0;
  uint32 entryId = 0;

  while (offset < end) {
    ext2_dir_entry* entry = (ext2_dir_entry*)ptr;
    TInode inode;

    Dir->AddEntry(*entry);

    uint entryInode = entry->inode - 1;

    GetOrReadInodeTable(entryInode, inode);

    uint group = entryInode / Super.InodesPerGroup;
    uint inodeOffset = entryInode % Super.InodesPerGroup;

    unique_ptr<CInodeReader>  inodeReader(CInodeReader::Create(
                                        &inode, Params.BlockSize));

    if (!BlockGroups[group]->IsInodeFree(inodeOffset)) {

      if (entryId == 0) {
        if ((!inodeReader->IsFolderInode()) ||
            (strcmp(entry->name, ".") != 0)) {
          throw CError(L"Not dot inode expected of the first in the folder");
        }
        fs_assert_eq(entry->inode - 1, Dir->GetIndex());
      }
      else if (entryId == 1) {
        if ((!inodeReader->IsFolderInode()) ||
            (strcmp(entry->name, "..") != 0)) {
          throw CError(L"Not dot inode expected of the first in the folder");
        }
        /* If the directory dot inode is 0, this could be a root directory */
        fs_assert_eq(entry->inode - 1, Dir->DotInode);
      }
      inodeReader->SetIndex(entryInode);

      if (entryId >= 2) {
        BlockGroups[group]->AddInodeWithSpecificNumber((CInode*)inodeReader.get(),
                                                   entryInode);
      }

      if (inodeReader->IsFolderInode() && (entryId >= 2)) {
        inodeReader->Read(*BlockMan.get(), &_Drive);

        CFolderInodeReader* subFolder = (CFolderInodeReader*)inodeReader.get();

        subFolder->DotInode = Dir->GetIndex();
        subFolder->TwoDotInode = Dir->DotInode;

        /* Recursive call to this funciton */
        /* Do not recursive rebuild dot and 2-dot folder */
        RebuildFolder((CFolderInodeReader*)inodeReader.get());
      }      
    }

    ptr += entry->rec_len;
    offset += entry->rec_len;
    ++entryId;

    if (entryId == Dir->Inode.HardLinkCnt) {
      break;
    }    
  }

}
//---------------------------------------------------------------------------
void CExt3Fsck::RebuildJournalInode()
{
  TInode inode;

  uint inodeIdx = EXT2_JOURNAL_INO - 1;

  GetOrReadInodeTable(inodeIdx, inode);

  unique_ptr<CJournalInodeReader>  journalReader(
                      new CJournalInodeReader(&inode, Params.BlockSize));

  BlockGroups[0]->AddInodeWithSpecificNumber((CInode*)journalReader.get(),
                                             inodeIdx);

  journalReader->SetIndex(inodeIdx);
  journalReader->Read(*BlockMan.get(), &_Drive);
  journalReader->CheckJournalSuperBlock();
}
//---------------------------------------------------------------------------
/*
 *  Get inode table from inode blocks buffer or read from media if not exists in buffer.
 */
void CExt3Fsck::GetOrReadInodeTable(uint32 InodeId, TInode& Inode)
{
  uint32 inodesPerBlock = Params.BlockSize / Params.InodeSize;
  uint32 inodeGroup = InodeId / Super.InodesPerGroup;

  TGroupDesc& desc = BlockGroups[inodeGroup]->Desc;

  uint32 inodeOffsetInGp = InodeId % Super.InodesPerGroup;
  uint32 inodeBlock = desc.InodeTableBlock + inodeOffsetInGp / inodesPerBlock;
  uint32 inodeOffsetInBlock = inodeOffsetInGp % inodesPerBlock;

  Bulk<byte>* buffer = NULL;

  if (_BlockBmpLoaded && (!BlockMan->HasBlockOccupied(inodeBlock))) {
    ////BlockMan->OccupyBlock(inodeBlock, 1);
    throw CError(L"The inode block was set to free in the block bitmap");
  }

  if (buffer == NULL) {
    ////throw CError(L"The occupied block's buffer is expected be initialized");
    buffer = BlockMan->CreateSingleBlockDataBuffer(inodeBlock);
    ReadBlocksToBuffer(inodeBlock, 1, *buffer);
  }
  
  TInode* inodes = (TInode*)(buffer->Data() + inodeOffsetInBlock * Params.InodeSize);

  Inode = *inodes;
}
//---------------------------------------------------------------------------
/*
 *  Read inode table blocks.
 */
void CExt3Fsck::ReadGroupInodeTable(uint32 Group)
{
  uint32 inodesPerBlock = Params.BlockSize / Params.InodeSize;  
  TGroupDesc& desc = BlockGroups[Group]->Desc;
  uint32 inodeCount = Super.InodesPerGroup;
  uint32 inodeBlockCnt = Super.InodesPerGroup * Super.InodeSize;
  uint32 endInodeBlock = desc.InodeTableBlock + inodeBlockCnt; 
  uint32 inodeOffset = Group * Super.InodesPerGroup;

  std::vector<byte>& inodeBmp = BlockGroups[Group]->GetInodeBmp();

  for (uint32 inode = 0; inode < inodeCount; ++inode) {
    if (inodeBmp[inode / 8] & (0x01 << (inode % 8))) {
      uint32 inodeTableBlock = desc.InodeBmpBlock + inodesPerBlock / inode;
      uint32 inodeTableOffset = inodesPerBlock % inode;

      Bulk<byte>* buffer = BlockMan->GetSingleBlockDataBuffer(inodeTableBlock);

      if (buffer == NULL) {
        buffer = BlockMan->CreateSingleBlockDataBuffer(inodeTableBlock);                        
        ReadBlocksToBuffer(inodeTableBlock, 1, *buffer);
      }
      TInode* inode = (TInode*)buffer->Data();

      
      
    }
  }
}
//---------------------------------------------------------------------------
/*
 *  Read block data from media to buffer
 */
void CExt3Fsck::ReadBlocksToBuffer(uint32 Block, uint32 BlockCount, Bulk<byte>& Buffer)
{
  uint32 end = Block + BlockCount;

  for (uint block = Block; block < end; ++block) {
    if (_BlockBmpLoaded && (!BlockMan->HasBlockOccupied(block))) {
      throw CError(L"The block to rebuild file system was set to free"
                   L" in the block bmp");
    }
  }
  assert(Buffer.Size() >= (Params.BlockSize * BlockCount));

  uint32 sectorsPerBlock = Params.BlockSize / 512;
  _Drive.ReadSector(Block * sectorsPerBlock, sectorsPerBlock * BlockCount, Buffer.Data());
}
//---------------------------------------------------------------------------
