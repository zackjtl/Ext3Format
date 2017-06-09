#include <cmath>
#include "Ext3Fs.h"
#include "GlobalDef.h"
#include "Tables.h"
#include "my_uuid.h"

CExt3Fs::CExt3Fs(uint64 TotalSectors)
  : TotalSectors(TotalSectors),
    Params(16384)
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
}

void CExt3Fs::CreateBlockManager()
{
  BlockMan.reset(new CBlockManager(Params.TotalBlocks, Params.BlockSize));  
}

void CExt3Fs::CreateBlockGroups()
{
	for (int i = 0; i < Params.GroupCount; ++i) {
  	CBlockGroup* ptr = new CBlockGroup(i, Super, Params, *BlockMan.get());

		ptr->OccupyFileSystemBlocks();
		ptr->CreateRsvdGdtBlockData();

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
  Super.FsCreateTime = timestamp;

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

  CalcReservedGdtBlocks();  

  /* Extended Super Block Section */
  Super.FirstNonReservedInode = 11; //??
  /* TODO: Check how Inode size was decided ? */
  Super.InodeSize = 256;    
  /* The super block's block */
  Super.SPIndex = Params.BlockSize <= 1024 ? 1 : 0;
}

/* 
 * Calculate GDT Reserved Block Count
 */
void CExt3Fs::CalcReservedGdtBlocks()
{
  uint64 rsv_blks = Params.TotalBlocks;

  /* Multiple 1024 and can't bigger than 0xFFFFFFFF */
  for (int i = 10; i; --i) {
    if (rsv_blks & 0x80000000 == 0) 
      rsv_blks <<= 1;    
    else
      break;
  }
  uint32 rsvd_groups = div_ceil(rsv_blks - Super.FirstDataBlk, Super.BlocksPerGroup);
  uint32 rsvd_gdb = div_ceil(rsvd_groups, Params.GroupDescPerBlock) - Params.GroupDescBlockCnt;  

  if ((rsvd_gdb / 4) > Params.BlockSize) {
    rsvd_gdb = Params.BlockSize / 4;
  }
  Super.ReservedBlockCnt = rsvd_gdb;

  /* This field is reserved in ext3 and older */
  Super.RsvdGdtBlocks = 0;   
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

  Super.InodeCnt = inodes_per_group * Params.GroupCount;
  Super.InodesPerGroup = inodes_per_group;  
  Super.FreeInodeCnt = Super.InodeCnt;
}



