#include <cmath>
#include "Ext3Fs.h"
#include "e3fs_def.h"
#include "e3fs_tables.h"
#include "e3fs_uuid.h"
#include "ResizeInode.h"
#include "JournalInode.h"
#include "BaseError.h"
#include "TypeConv.h"
#include "E2fsBlockIo.h"

/*
 *  The default constructor is used for create an empty one to
 *  read fs from media.
 */
CExt3Fs::CExt3Fs()
{
}

CExt3Fs::CExt3Fs(uint64 TotalSectors)
  : TotalSectors(TotalSectors),
    Params(TotalSectors)
{
  _HasJournal = true;
}

CExt3Fs::~CExt3Fs()
{
}

void CExt3Fs::Create()
{
	srand(time(NULL));

	InitSuperBlock();
  CreateBlockManager();  
	CreateBlockGroups();

	/* Create file system specific inodes */
	CreateRootDirectory();
  CreateResizeInode();
  CreateJournalInode();
  CreateSpecialInodes();
  CreateLostAndFoundDirectory();  

  /* Sync block bitmap and update group descriptors of each groups */
  UpdateBlockGroupsInfo();
  UpdateInodeTables();
  UpdateSuperBlock();

	/* Flush file system tables into buffer */
	FlushGroupDescData();
	FlushBlockBmp();
	FlushInodeBmp();
	FlushInodeTables();
	FlushSuperBlock();
}

void CExt3Fs::CreateBlockManager()
{
  BlockMan.reset(new CBlockManager(Params.TotalBlocks, Params.BlockSize));  
}

void CExt3Fs::CreateBlockGroups()
{
	for (uint32 i = 0; i < Params.GroupCount; ++i) {
  	CBlockGroup* ptr = new CBlockGroup(i, Super, Params, *BlockMan.get());
    ptr->Initial();
		ptr->OccupyFileSystemBlocks();
		BlockGroups.push_back(ptr);
  }
}

void CExt3Fs::InitSuperBlock()
{
  Super.BlocksPerGroup = Params.BlocksPerGroup;
  Super.FirstDataBlk = 0;
  Super.FragmentsPerGroup = Super.BlocksPerGroup;
	Super.BlockSizeFlag = log((double)Params.BlockSize / 1024) / log((double)2);
	Super.FragmentSize = Super.BlockSizeFlag;
  
  Super.BlockCnt = (uint32)Params.TotalBlocks; 
  Super.BlockCountHi = (uint32)(Params.TotalBlocks >> 32);
  Super.FreeBlockCnt = Super.BlockCnt;
	Super.FreeBlockCountHi = Super.BlockCountHi;

  uint32 sectorsPerBlock = Params.BlockSize / 512;
  uint32 sbStartSector = Params.MBRSize / 512;
  Params.BlockOfSuperBlock = sbStartSector / sectorsPerBlock;

  Super.Magic[0] = 0x53;
  Super.Magic[1] = 0xef;

  gen_uuid_v4(Super.UUIDOfVolume, 16);
  memset((byte*)&Super.VolumeName[0], 0, sizeof(Super.VolumeName));
  memset((byte*)&Super.LastMountedPath[0], 0, sizeof(Super.LastMountedPath));

  memset((byte*)&Super.JournalUUID[0], 0, sizeof(Super.JournalUUID));

  /* TODO: Check how Inode size was decided ? */
  Super.InodeSize = 256;  

	CalcInodeNumber();

  /* Basically is 32 */
  uint32 group_desc_size = sizeof(TGroupDesc); 

  Super.LastMountTime = 0;  
  Super.MaxMountCnt = 0xFFFF;
  Super.MountCnt = 0;
  Super.FileSysState = 1; /* ?? */
  Super.BehaviorOnErr = 1; /* ?? */

  /* OS = Linux */
  Super.MaxTimeBetweenChecks = 0;
  Super.OS = 0;
  Super.RevLevel = 1;
  Super.UsrIdForRevBlks = 0;
  Super.GroupIdForRevBlks = 0;

	CalcReservedBlocks();
  CalcReservedGdtBlocks();  

  /* Extended Super Block Section */
	Super.FirstNonReservedInode = EXT2_GOOD_OLD_FIRST_INO; //??
  /* The super block's block */
  Super.SPIndex = Params.BlockSize <= 1024 ? 1 : 0;

  Super.CompFeatureFlags = 0;
  Super.CompFeatureFlags |= EXT2_FEATURE_COMPAT_RESIZE_INODE |
                            EXT2_FEATURE_COMPAT_DIR_INDEX |
                            EXT3_FEATURE_COMPAT_HAS_JOURNAL |
                            EXT2_FEATURE_COMPAT_EXT_ATTR;
  Super.ROCompFeatureFlags = 0;
  Super.ROCompFeatureFlags |= EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER |
                              EXT2_FEATURE_RO_COMPAT_LARGE_FILE;
  Super.IncompFeatureFlags = 0;
  Super.IncompFeatureFlags |= EXT2_FEATURE_INCOMPAT_FILETYPE;

  Super.AlgorithmUsageBmp = 0;
  Super.BlocksPreAlloc = 0;
  Super.DirBlocksPreAlloc = 0;  

  Super.JournalInode = 0;
  Super.JournalDevice = 0;
  Super.LastOrphanedInode = 0;

  byte* pHashSeed = (byte*)&Super.HashSeed[0];

  gen_uuid_v4(pHashSeed, 16);

  Super.DefaultHashVersion = 1;
	Super.DefaultMountOptions = 12;  

  Super.FirstMetaBlockGroup = 0;
  Super.FsCreateTime = GetPosixTime();

  Super.JournalInode = EXT2_JOURNAL_INO;

  Super.MinExtraISize = 28;
  Super.WantExtraISize = 28;
  Super.MiscFlags = 0;
  Super.MiscFlags |= EXT2_FLAGS_UNSIGNED_HASH;  
}

/* 
 * Calculate Reserved Block Count with a reserved ratio (default 5%)
 */
void CExt3Fs::CalcReservedBlocks()
{
  Super.ReservedBlockCnt = (Super.BlockCnt / 100) * Params.ReservedPercent;
}

/* 
 * Calculate GDT Reserved Block Count
 */
void CExt3Fs::CalcReservedGdtBlocks()
{
	uint64 max_blocks = 0xffffffff;

  if (Super.BlockCnt < (max_blocks / 1024)) {
    max_blocks = Super.BlockCnt * 1024;
	}

	uint32 rsvd_groups = div_ceil(max_blocks - Super.FirstDataBlk, Super.BlocksPerGroup);
	uint32 rsvd_gdb = div_ceil(rsvd_groups, Params.GroupDescPerBlock) - Params.GroupDescBlockCnt;

  if ((rsvd_gdb > (Params.BlockSize / sizeof(uint32)))){
    rsvd_gdb = (Params.BlockSize / sizeof(uint32));
  }
  Super.RsvdGdtBlocks = rsvd_gdb;
}

/* 
 *  Synchronouse block bitmap and group descriptors of each groups.
 */
void CExt3Fs::UpdateBlockGroupsInfo()
{
	for (uint32 gp = 0; gp < Params.GroupCount; ++gp) {
		BlockGroups[gp]->UpdateGroupInfo(*BlockMan.get());
  }
}

/* 
 *  Update inode tables of each groups.
 */
void CExt3Fs::UpdateInodeTables()
{
	for (uint32 gp = 0; gp < Params.GroupCount; ++gp) {
    BlockGroups[gp]->UpdateInodeTables();
  }
}

/*
 *  Update super block informations from the groups
 */
void CExt3Fs::UpdateSuperBlock()
{
  uint64 freeBlocks = 0;
  Super.FreeInodeCnt = 0;

	for (uint32 gp = 0; gp < Params.GroupCount; ++gp) {
    freeBlocks += BlockGroups[gp]->Desc.FreeBlockCount;
    Super.FreeInodeCnt += BlockGroups[gp]->Desc.FreeInodeCount;
  }
  Super.FreeBlockCnt = freeBlocks;
  Super.FreeBlockCountHi = freeBlocks >> 32;

  Super.LastWriteTime = GetPosixTime();
  Super.TimeOfLastCheck = GetPosixTime();
}


#define occupied_wo_write(x) (BlockMan->HasBlockOccupied(x) && (!BlockMan->HasBlockWritten(x)))

/*
 *  Set data into the group descript blocks
 */
void CExt3Fs::FlushGroupDescData()
{
  Bulk<byte>  buffer(Params.GroupDescBlockCnt * Params.BlockSize);
  memset(buffer.Data(), 0, buffer.Size());

  TGroupDesc* desc_ptr = (TGroupDesc*)buffer.Data();

  assert(BlockGroups.size() == Params.GroupCount);

	for (uint32 gp = 0; gp < Params.GroupCount; ++gp) {
    *desc_ptr++ = BlockGroups[gp]->Desc;  
  }

	for (uint32 gp = 0; gp < Params.GroupCount; ++gp) {
		if (CBlockGroup::bg_has_super(Super, gp)) {
			uint32 start_desc_block = BlockGroups[gp]->SuperBlockOffset + 1;

			for (uint32 i = 0; i <Params.GroupDescBlockCnt; ++i) {
				uint32 group_desc_block = start_desc_block + i;

				if (!occupied_wo_write(group_desc_block)) {
					throw CError(L"The group descriptor block had not allocated or has been written");
				}
			}
			BlockMan->SetBlockData(start_desc_block, buffer.Data(), buffer.Size());
    }
  }
}

/*
 *  Set data into block bmp blocks of each groups
 */
void CExt3Fs::FlushBlockBmp()
{
	for (uint32 gp = 0; gp < Params.GroupCount; ++gp) {
		uint32 blockBmpBlk = BlockGroups[gp]->Desc.BlockBmpBlock;
		if (!occupied_wo_write(blockBmpBlk))
			throw CError(L"The group descriptor block had not allocated or has been written");

		vector<byte>& blockBmp = BlockGroups[gp]->GetBlockBmp();
		Bulk<byte>* buffer = BlockMan->CreateSingleBlockDataBuffer(blockBmpBlk);

		if (buffer != NULL) {
			memset(buffer->Data(), 0x00, buffer->Size());
			memcpy(buffer->Data(), (byte*)&blockBmp[0], blockBmp.size());
		}
  }  
}

/*
 *  Set data into inode bmp blocks of each groups
 */
void CExt3Fs::FlushInodeBmp()
{
	for (uint32 gp = 0; gp < Params.GroupCount; ++gp) {
		uint32 inodeBmpBlk = BlockGroups[gp]->Desc.InodeBmpBlock;
		if (!occupied_wo_write(inodeBmpBlk))
			throw CError(L"The group descriptor block had not allocated or has been written");

		vector<byte>& inodeBmp = BlockGroups[gp]->GetInodeBmp();
		Bulk<byte>* buffer = BlockMan->CreateSingleBlockDataBuffer(inodeBmpBlk);

		if (buffer != NULL) {
			memset(buffer->Data(), 0x00, buffer->Size());
			memcpy(buffer->Data(), (byte*)&inodeBmp[0], inodeBmp.size());
		}
  }
}

/*
 *  Set inode table data into each groups
 */
void CExt3Fs::FlushInodeTables()
{
	for (uint32 gp = 0; gp < Params.GroupCount; ++gp) {
    BlockGroups[gp]->FlushInodeTables(*BlockMan.get());
  }
}

/*
 *  Set MBR and the super block table in each groups backup
 */
void CExt3Fs::FlushSuperBlock()
{
	for (uint32 gp = 0; gp < Params.GroupCount; ++gp) {
		if (CBlockGroup::bg_has_super(Super, gp)) {
      uint gpDescBlock = gp * Params.BlocksPerGroup +
                          (gp == 0 ? Params.BlockOfSuperBlock : 0);

			Bulk<byte>*buffer = BlockMan->CreateSingleBlockDataBuffer(
                            gp * Params.BlocksPerGroup);
			memset(buffer->Data(), 0x00, buffer->Size());

      byte* ptr = buffer->Data();

      if (gp == 0)
        ptr += Params.MBRSize;

			TSuperBlock* ptable = (TSuperBlock*)ptr;
			*ptable = Super;
      ptable->SPIndex = gp;
		}
	}
}

/* 
 *  calculate Inode count
 */
void CExt3Fs::CalcInodeNumber()
{
	/*
	 * Make sure the number of inodes per group completely fills
	 * the inode table blocks in the descriptor.  If not, add some
	 * additional inodes/group.  Waste not, want not...
	 */
  uint32 rough_inodes = ((float)Params.TotalBlocks / Params.InodeRatio) * Params.BlockSize;
  uint32 inodes_per_group = div_ceil(rough_inodes, Params.GroupCount);
  uint32 inodes_per_block = Params.BlockSize / Super.InodeSize;
  InodeBlocksPerGroup = div_ceil(inodes_per_group * Super.InodeSize, Params.BlockSize);

  inodes_per_group = inodes_per_block * InodeBlocksPerGroup;

	/*
	 * Finally, make sure the number of inodes per group is a
	 * multiple of 8.  This is needed to simplify the bitmap
	 * splicing code.
	 */
	if (inodes_per_group < 8)
		inodes_per_group = 8;
	inodes_per_group &= ~7;  

  InodeBlocksPerGroup = div_ceil(inodes_per_group * Super.InodeSize, Params.BlockSize);

  Params.InodeBlocksPerGroup = InodeBlocksPerGroup; 

  Super.InodeCnt = inodes_per_group * Params.GroupCount;
  Super.InodesPerGroup = inodes_per_group;  
  Super.FreeInodeCnt = Super.InodeCnt;  
}

/* 
 *  Create root directory
 */
void CExt3Fs::CreateRootDirectory()
{
	CBlockGroup* bg0 = BlockGroups[0];

	RootInode = (CFolderInode*)CInode::Create(LINUX_S_IFDIR, Params.BlockSize);

	if (!bg0->AddInodeWithSpecificNumber(RootInode, EXT2_ROOT_INO - 1)) {
		throw CError(L"Allocate inode for root directory failed");
	}
	RootInode->SetIndex(1);
  RootInode->SetName("//");
  RootInode->MkDir(1, 1, *BlockMan.get(), Super);
  RootInode->SetPermissions(755);
}

/* 
 *  Create resize inode
 */
void CExt3Fs::CreateResizeInode()
{
  CBlockGroup* gb0 = BlockGroups[0];

  CResizeInode* inode = new CResizeInode(Params.BlockSize);

  if (!gb0->AddInodeWithSpecificNumber(inode, EXT2_RESIZE_INO - 1)) {
    throw CError(L"Allocate resize inode failed");
  }
  inode->SetData(*BlockMan.get(), Super, Params);
}

/* 
 *  Create journal inode
 */
void CExt3Fs::CreateJournalInode()
{
  int journalBlocks = GetDefaultJournalBlocks();

  if (journalBlocks <= 0) {
    Super.CompFeatureFlags &= (~EXT3_FEATURE_COMPAT_HAS_JOURNAL);
    _HasJournal = false;
    return;
  }

  ShiftBlockAllocPtrForJournal();  

  CBlockGroup* gb0 = BlockGroups[0];
  CJournalInode* inode = new CJournalInode(Params.BlockSize, (uint32)journalBlocks);

  if (!gb0->AddInodeWithSpecificNumber(inode, EXT2_JOURNAL_INO - 1)) {
    throw CError(L"Allocate journal inode failed");
  }
  inode->SetData(*BlockMan.get(), Super, Params);

  memcpy((byte*)&Super.JournalBlock[0], (byte*)&inode->Inode.Blocks[0], sizeof(inode->Inode.Blocks));
  Super.JournalFileSize = (uint32)inode->GetSize();

  RestoreBlockAllocPtr();
}

int CExt3Fs::GetDefaultJournalBlocks()
{
  if (Params.TotalBlocks < 2048)       /* < 8MB */
      return -1;
  if (Params.TotalBlocks < 32768)      /* < 128MB */
      return (1024);
  if (Params.TotalBlocks < 256*1024)   /* < 1GB */
      return (4096);           
  if (Params.TotalBlocks < 512*1024)   /* < 2GB */
      return (8192);
  if (Params.TotalBlocks < 1024*1024)  /* < 4GB */
      return (16384);
  return 32768;
}


/* 
 *  Create lost+found directory
 */
void CExt3Fs::CreateLostAndFoundDirectory()
{
  CBlockGroup* gb0 = BlockGroups[0];

	CInode* inode = gb0->AllocateNewInode(LINUX_S_IFDIR);

	inode->SetIndex(EXT2_GOOD_OLD_FIRST_INO - 1);
	inode->SetName("lost+found");
  inode->SetPermissions(700);

	RootInode->Attach(inode, *BlockMan.get(), Super);

  uint32 block_cnt = 16384 / Params.BlockSize;

	uint32 offset = inode->GetSize();
	uint32 remain = 16384 - offset;
	uint32 remain_blocks = div_ceil(remain, Params.BlockSize);

	Bulk<byte> buffer(remain);

	memset(buffer.Data(), 0x00, buffer.Size());

	for (uint32 block = 1; block < remain_blocks; ++block) {
		byte* ptr = buffer.Data() + (block * Params.BlockSize - offset);
		/* Don't know why but according to the formatted card */
		ptr[5] = 0x10;
	}
	inode->SetData(*BlockMan.get(), buffer.Data(), buffer.Size());
}


/*
 *  In the mke2fs, the blocks to store journal was shifted to the group
 *  of the half of total groups.
 */
void CExt3Fs::ShiftBlockAllocPtrForJournal()
{
  uint32 journalLocatedBg = (Params.GroupCount / 2) - 1;

  while (CBlockGroup::bg_has_super(Super, journalLocatedBg)) {
    --journalLocatedBg;
  }

  _BlockAllocPtrBackup = BlockMan->GetBasePtr();  

  BlockMan->ShiftBasePtr(journalLocatedBg * Params.BlocksPerGroup);
}

void CExt3Fs::RestoreBlockAllocPtr()
{
  BlockMan->ShiftBasePtr(_BlockAllocPtrBackup);
}

/* 
 *  Create the others special inodes
 */
void CExt3Fs::CreateSpecialInodes()
{
  CBlockGroup* gb0 = BlockGroups[0];

  for (uint i = 0; i < (EXT2_GOOD_OLD_FIRST_INO - 1); ++i) {
    if (!gb0->IsInodeExists(i)) {
      CInode* inode = gb0->AllocateNewInode(0);
      inode->SetIndex(i);
    }
  }
}

/*
 *	Real write file system into the storage.
 */
void CExt3Fs::Write(CUsbDrive& Drive)
{
  CE2fsBlockIo io(*BlockMan.get(), &Drive, false);

  io.SetQueueBmp(BlockMan->GetWrittenBmp());
  io.WriteBlocksInQueue();
}


