#include <cmath>
#include "Ext3Fs.h"
#include "GlobalDef.h"
#include "Tables.h"
#include "my_uuid.h"
#include "ResizeInode.h"
#include "BaseError.h"

CExt3Fs::CExt3Fs(uint64 TotalSectors)
  : TotalSectors(TotalSectors),
    Params(TotalSectors)
{
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
	CreateLostAndFoundDirectory();
  CreateResizeInode();

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

  Super.Magic[0] = 0x53;
  Super.Magic[1] = 0xef;

  gen_uuid_v4(Super.UUIDOfVolume, 16);
  memset((byte*)&Super.VolumeName[0], 0, sizeof(Super.VolumeName));
  memset((byte*)&Super.LastMountedPath[0], 0, sizeof(Super.LastMountedPath));

  memset((byte*)&Super.JournalUUID[0], 0, sizeof(Super.JournalUUID));

  srand(time(NULL));

  Super.HashSeed[0] = rand();
  Super.HashSeed[1] = rand();
  Super.HashSeed[2] = rand();
	Super.HashSeed[3] = rand();

  Super.DefaultHashVersion = 1;
	Super.DefaultMountOptions = 12;

	std::time_t timestamp = std::time(NULL);
  Super.LastMountTime = 0;
  Super.FsCreateTime = timestamp;

  /* TODO: Check how Inode size was decided ? */
  Super.InodeSize = 256;

	CalcInodeNumber();

  /* Basically is 32 */
  uint32 group_desc_size = sizeof(TGroupDesc); 

  //Super.MountCnt = ?? 
  Super.MaxMountCnt = 0xFFFF;

  /* OS = Linux */
  Super.MaxTimeBetweenChecks = 0;
  Super.OS = 0;
  Super.RevLevel = 1;
  Super.UsrIdForRevBlks = 0;
  Super.GroupIdForRevBlks = 0;

	CalcReservedBlocks();
  CalcReservedGdtBlocks();  

  /* Extended Super Block Section */
	Super.FirstNonReservedInode = 11; //??
  /* The super block's block */
  Super.SPIndex = Params.BlockSize <= 1024 ? 1 : 0;

  Super.CompFeatureFlags = 0;
  Super.CompFeatureFlags |= EXT2_FEATURE_COMPAT_RESIZE_INODE |
                            EXT2_FEATURE_COMPAT_DIR_INDEX;
  Super.ROCompFeatureFlags = 0;
  Super.ROCompFeatureFlags |= EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER |
                              EXT2_FEATURE_RO_COMPAT_LARGE_FILE;
  Super.IncompFeatureFlags = 0;
  Super.IncompFeatureFlags |= EXT2_FEATURE_INCOMPAT_FILETYPE;
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
  
  time((long*)&Super.LastWriteTime);
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
			uint32 start_desc_block = BlockGroups[gp]->StartBlock + 1;

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
			Bulk<byte>*buffer = BlockMan->CreateSingleBlockDataBuffer(
                            gp * Params.BlocksPerGroup);
			memset(buffer->Data(), 0x00, buffer->Size());

      byte* ptr = buffer->Data();

      if (gp == 0)
        ptr += CExt2Params::MBRSize;

			TSuperBlock* ptable = (TSuperBlock*)ptr;
			*ptable = Super;
      ptable->SPIndex = gp;

      int temp = gp;
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
  uint32 rough_inodes = (Params.TotalBlocks / CExt2Params::InodeRatio) * CExt2Params::BlockSize;
  uint32 inodes_per_group = div_ceil(rough_inodes, Params.GroupCount);    
  InodeBlocksPerGroup = div_ceil(inodes_per_group * Super.InodeSize, CExt2Params::BlockSize);
  inodes_per_group = (InodeBlocksPerGroup * CExt2Params::BlockSize) / Super.InodeSize;

	/*
	 * Finally, make sure the number of inodes per group is a
	 * multiple of 8.  This is needed to simplify the bitmap
	 * splicing code.
	 */
	if (inodes_per_group < 8)
		inodes_per_group = 8;
	inodes_per_group &= ~7;  

  InodeBlocksPerGroup = div_ceil(inodes_per_group * Super.InodeSize, CExt2Params::BlockSize);

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

	RootInode = (CFolderInode*)CInode::Create(EXT2_FT_DIR, Params.BlockSize);

	if (!bg0->OccupyInodeNumber(RootInode, EXT2_ROOT_INO - 1)) {
		throw CError(L"Allocate inode for root directory failed");
	}
	RootInode->SetIndex(1);
  RootInode->SetName("//");
  RootInode->MkDir(1, 1, *BlockMan.get(), Super);
}

/* 
 *  Create resize inode
 */
void CExt3Fs::CreateResizeInode()
{
  CBlockGroup* gb0 = BlockGroups[0];

  CResizeInode* inode = new CResizeInode(Params.BlockSize);

  if (!gb0->OccupyInodeNumber(inode, EXT2_RESIZE_INO - 1)) {
    throw CError(L"Allocate resize inode failed");
  }
  inode->WriteData(*BlockMan.get(), Super, Params);
}

/* 
 *  Create lost+found directory
 */
void CExt3Fs::CreateLostAndFoundDirectory()
{
  CBlockGroup* gb0 = BlockGroups[0];

	CInode* inode = gb0->AllocateNewInode(EXT2_FT_DIR);

	inode->SetIndex(EXT2_GOOD_OLD_FIRST_INO - 1);
	inode->SetName("lost+found");

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
	inode->WriteData(*BlockMan.get(), buffer.Data(), buffer.Size());
}

/*
 *	Real write file system into the storage.
 */
void CExt3Fs::Write(CUsbDrive& Drive)
{
	#define used_test(x) (used_bmp[x / 8] & (0x01 << (x % 8)))
	#define written_test(x) (written_bmp[x / 8] & (0x01 << (x % 8)))
	#define wr_valid(x) (used_test(x) && written_test(x))

	const uint32 frag_window = 4;
	vector<byte>&	used_bmp = BlockMan->GetBlockBmp();
	vector<byte>&	written_bmp = BlockMan->GetWrittenBmp();

	uint32 window_cnt = div_ceil(Super.BlockCnt, frag_window);
	uint32 remain = Super.BlockCnt;

	for (uint32 window = 0; window < window_cnt; ++window) {
		uint32 offset = window * frag_window;
		uint32 end = offset + frag_window;

		uint32 cont_start = 0;
		uint32 cont_cnt = 0;
		bool previous = false;

		for (uint32 block = offset; block < end; ++block) {
			if (wr_valid(block)) {
				if (previous) {
					cont_cnt++;
				}
				else {
					cont_start = block;
					cont_cnt = 1;
					previous = true;
				}
			}
			else {
				if (cont_cnt) {
					// Write continuous area..
         	WriteArea(Drive, cont_start, cont_cnt);
					cont_cnt = 0;
					previous  = false;
				}

      	previous = false;
			}
		}
		if (cont_cnt) {
			// Write continuous area..
			WriteArea(Drive, cont_start, cont_cnt);
		}
	}
}

/*
 *	Write continuous area within a specific fragment length
 */
void CExt3Fs::WriteArea(CUsbDrive& Drive, uint32 StartBlock, uint32 Count)
{
	Bulk<byte>	buffer(Count * Params.BlockSize);

	BlockMan->GetBlockData(StartBlock, buffer.Data(), buffer.Size());

	uint32 sectorsPerBlock = Params.BlockSize / 512;
	uint32 sector_pos = StartBlock * sectorsPerBlock;
	uint32 sector_cnt = Count * sectorsPerBlock;

	Drive.WriteSector(sector_pos, sector_cnt, buffer.Data());
}



