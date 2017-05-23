#ifndef TablesH
#define TablesH

#define "Types.h"

/*
 * Constants relative to the data blocks
 */
#define EXT2_NDIR_BLOCKS		12
#define EXT2_IND_BLOCK			EXT2_NDIR_BLOCKS
#define EXT2_DIND_BLOCK			(EXT2_IND_BLOCK + 1)
#define EXT2_TIND_BLOCK			(EXT2_DIND_BLOCK + 1)
#define EXT2_N_BLOCKS			  (EXT2_TIND_BLOCK + 1)

#define JFS_MAGIC_NUMBER 0xc03b3998U /* The first 4 bytes of /dev/random! */

#pragma pack(push) /* push current alignment to stack */
#pragma pack(1) /* set alignment to 1 byte boundary */

/*
 *   Super Block Table
 */
typedef struct ext2_super_block {
  uint32  InodeCnt;
  uint32  BlockCnt;
  uint32  ReservedBlockCnt;
  uint32  FreeBlockCnt;
  uint32  FreeInodeCnt;
  uint32  FirstDataBlk;
  uint32  BlockSize;
  uint32  FragmentSize;     //Cluser Size
  uint32  BlocksPerGroup;
  uint32  FragmentsPerGroup;
  uint32  InodesPerGroup;
  // TODO: Redefine to UNIX time structure
  uint32  LastMountTime;
  uint32  LastWriteTime;
  uint16  MountCnt;
  uint16  MaxMountCnt;      // default: 0xFFFF
  uint8   Magic[2];         // default: 0x53EF
  uint16  FileSysState;     // 1 ?
  uint16  BehaviorOnErr;    // 1 ?
  uint16  MinorRevLevel;    // 0 ?
  uint32  TimeOfLastCheck;  
  uint32  MaxTimeBetweenChecks; // 0 ?
  uint32  OS;   // 0: Linux
  uint32  RevLevel; //  1 ?
  uint16  UsrIdForRevBlks;    //  0 ?
  uint16  GroupIdForRevBlks;  //  0 ?
  
  /* Extended Super Block Section */
  uint32  FirstNonReservedInode;  // Usually be 11
  uint16  InodeSize;              // 128/256/512...
  uint16  SPIndex;                // This super block's block (index)  
  uint32  CompFeatureFlags;       // Compability Feature Flags
  uint32  IncompFeatureFlags;     // Incompability Feature Flags
  uint32  ROCompFeatureFlags;     // RO-Compability Feature Flags
  uint8   UUIDOfVolume[16];
  char    VolumeName[16];
  char    LastMountedPath[64];
  uint32  AlgorithmUsageBmp;
  uint8   BlocksPreAlloc;
  uint8   DirBlocksPreAlloc;
  uint16  RsvdGdtBlocks;
  uint8   JournalUUID[16];
  uint32  JournalInode;
  uint32  JournalDevice;
  uint32  LastOrphanedInode;
  uint32  HashSeed[4];
  uint32  DefaultHashVersion;   // 1 ?  
  uint32  DefaultMountOptions;  // 12 ?
  uint32  FirstMetaBlockGroup;
  uint32  FsCreateTime;

  /* Journal Inode Backup */
  uint32  JournalBlock[12];
  uint32  JournalIBlock;      // Journal Indirect Block
  uint32  JournalDIBlock;     // Journal Double Indirect
  uint32  JournalTIBlock;     // Journal Tripple Indirect Block
  uint32  Rsvd;
  uint32  JournalFileSize;  

  /* 64-Bit Support */
  uint32  BlockCountHi;
  uint32  RsvdBlockCountHi;
  uint32  FreeBlockCountHi;  
} TSuperBlock;

/*
 *   Group Descriptor   
 */
typedef struct ext2_group_desc
{
  uint32  BlockBmpBlock;
  uint32  InodeBmpBlock;
  uint32  InodeTableBlock;
  uint16  FreeBlockCount;
  uint16  FreeInodeCount;
  uint16  DirectoriesCount;
  uint16  Flags;
  uint32  SnapshotExcludeBmp;
  uint16  BlockBmpChecksum;
  uint16  InodeBmpChecksum;
  uint16  NeverUsedInodeCnt;
  uint16  GroupDescChecksum;  
}TGroupDesc;

/*
 *  inode structure
 */
typedef struct ext2_inode {
	uint16	Mode;		/* File mode */
	uint16	Uid;		/* Low 16 bits of Owner Uid */
	uint32	SizeInBytesLo;		/* Size in bytes */
	uint32	AccessTime;	/* Access time */
	uint32	InodeChangeTime;	/* Inode change time */
	uint32	ModificationTime;	/* Modification time */
	uint32	DeleteTime;	/* Deletion Time */
	uint16	GroupId;		/* Low 16 bits of Group Id */
	uint16	HardLinkCnt;	/* Links count */
	uint32	SectorCount;	/* Blocks count */
	uint32	FileFlags;	/* File flags */
	uint32  OS_Dep1;    /* OS Dependent1 */
	uint32	Blocks[EXT2_N_BLOCKS];/* Pointers to blocks */
	uint32	FileVersion;	/* File version (for NFS) */
	uint32	FileACL;	/* File ACL */
	uint32	SizeInBytesHi;	/* Formerly i_dir_acl, directory ACL */
	uint32	FragAddress;	/* Fragment address */
  /* OS Dependent 2 */
  uint8   FragNumber;
  uint8   FragSize;
  uint16  Padding;
  uint16  Rsvd[4];
}TInode;

#define JFS_DESCRIPTOR_BLOCK	1
#define JFS_COMMIT_BLOCK	2
#define JFS_SUPERBLOCK_V1	3
#define JFS_SUPERBLOCK_V2	4
#define JFS_REVOKE_BLOCK	5

/*
 * Standard header for all descriptor blocks:
 */
typedef struct journal_header_s
{
	uint32		Signature;
	uint32		BlockType;
	uint32		SequenceNumber;
} JournalHeader;

/*
 * The journal superblock
 */
typedef struct journal_superblock_s
{
	JournalHeader s_header;

	/* Static information describing the journal */
	uint32		BlockSize;	/* journal device blocksize */
	uint32		BlockCount;	/* total blocks in journal file */
	uint32		StartBlock;	/* first block of log information */

	/* Dynamic information describing the current state of the log */
	uint32		SequenceOf1stTrans;	/* first commit ID expected in log */
	uint32		BlockOf1stTrans;	/* blocknr of start of log */
} TJournalSuperBlock;

#pragma pack(pop) /* restore original alignment from stack */

#endif
