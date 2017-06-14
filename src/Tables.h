#ifndef TablesH
#define TablesH

#include "BaseTypes.h"

/*
 * Constants relative to the data blocks
 */
#define EXT2_NDIR_BLOCKS		12
#define EXT2_IND_BLOCK			EXT2_NDIR_BLOCKS
#define EXT2_DIND_BLOCK			(EXT2_IND_BLOCK + 1)
#define EXT2_TIND_BLOCK			(EXT2_DIND_BLOCK + 1)
#define EXT2_N_BLOCKS			  (EXT2_TIND_BLOCK + 1)

/*
 * Special inode numbers
 */
#define EXT2_BAD_INO		 1	/* Bad blocks inode */
#define EXT2_ROOT_INO		 2	/* Root inode */
#define EXT4_USR_QUOTA_INO	 3	/* User quota inode */
#define EXT4_GRP_QUOTA_INO	 4	/* Group quota inode */
#define EXT2_BOOT_LOADER_INO	 5	/* Boot loader inode */
#define EXT2_UNDEL_DIR_INO	 6	/* Undelete directory inode */
#define EXT2_RESIZE_INO		 7	/* Reserved group descriptors inode */
#define EXT2_JOURNAL_INO	 8	/* Journal inode */
#define EXT2_EXCLUDE_INO	 9	/* The "exclude" inode, for snapshots */
#define EXT4_REPLICA_INO	10	/* Used by non-upstream feature */

/* First non-reserved inode for old ext2 filesystems */
#define EXT2_GOOD_OLD_FIRST_INO	11


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
  uint32  BlockSizeFlag;
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

/*
 * Inode flags
 */
#define EXT2_SECRM_FL			0x00000001 /* Secure deletion */
#define EXT2_UNRM_FL			0x00000002 /* Undelete */
#define EXT2_COMPR_FL			0x00000004 /* Compress file */
#define EXT2_SYNC_FL			0x00000008 /* Synchronous updates */
#define EXT2_IMMUTABLE_FL		0x00000010 /* Immutable file */
#define EXT2_APPEND_FL			0x00000020 /* writes to file may only append */
#define EXT2_NODUMP_FL			0x00000040 /* do not dump file */
#define EXT2_NOATIME_FL			0x00000080 /* do not update atime */
/* Reserved for compression usage... */
#define EXT2_DIRTY_FL			0x00000100
#define EXT2_COMPRBLK_FL		0x00000200 /* One or more compressed clusters */
#define EXT2_NOCOMPR_FL			0x00000400 /* Access raw compressed data */
#define EXT2_ECOMPR_FL			0x00000800 /* Compression error */
/* End compression flags --- maybe not all used */
#define EXT2_BTREE_FL			0x00001000 /* btree format dir */
#define EXT2_INDEX_FL			0x00001000 /* hash-indexed directory */
#define EXT2_IMAGIC_FL			0x00002000
#define EXT3_JOURNAL_DATA_FL		0x00004000 /* file data should be journaled */
#define EXT2_NOTAIL_FL			0x00008000 /* file tail should not be merged */
#define EXT2_DIRSYNC_FL 		0x00010000 /* Synchronous directory modifications */
#define EXT2_TOPDIR_FL			0x00020000 /* Top of directory hierarchies*/
#define EXT4_HUGE_FILE_FL               0x00040000 /* Set to each huge file */
#define EXT4_EXTENTS_FL 		0x00080000 /* Inode uses extents */
#define EXT4_EA_INODE_FL	        0x00200000 /* Inode used for large EA */
/* EXT4_EOFBLOCKS_FL 0x00400000 was here */
#define FS_NOCOW_FL			0x00800000 /* Do not cow file */
#define EXT4_SNAPFILE_FL		0x01000000  /* Inode is a snapshot */
#define EXT4_SNAPFILE_DELETED_FL	0x04000000  /* Snapshot is being deleted */
#define EXT4_SNAPFILE_SHRUNK_FL		0x08000000  /* Snapshot shrink has completed */
#define EXT2_RESERVED_FL		0x80000000 /* reserved for ext2 lib */

#define EXT2_FL_USER_VISIBLE		0x004BDFFF /* User visible flags */
#define EXT2_FL_USER_MODIFIABLE		0x004B80FF /* User modifiable flags */

/*
 *  EXT2 Feature Flags
 */
#define EXT2_HAS_COMPAT_FEATURE(sb,mask)			\
	( EXT2_SB(sb)->s_feature_compat & (mask) )
#define EXT2_HAS_RO_COMPAT_FEATURE(sb,mask)			\
	( EXT2_SB(sb)->s_feature_ro_compat & (mask) )
#define EXT2_HAS_INCOMPAT_FEATURE(sb,mask)			\
	( EXT2_SB(sb)->s_feature_incompat & (mask) )

#define EXT2_FEATURE_COMPAT_DIR_PREALLOC	0x0001
#define EXT2_FEATURE_COMPAT_IMAGIC_INODES	0x0002
#define EXT3_FEATURE_COMPAT_HAS_JOURNAL		0x0004
#define EXT2_FEATURE_COMPAT_EXT_ATTR		0x0008
#define EXT2_FEATURE_COMPAT_RESIZE_INODE	0x0010
#define EXT2_FEATURE_COMPAT_DIR_INDEX		0x0020
#define EXT2_FEATURE_COMPAT_LAZY_BG		0x0040
/* #define EXT2_FEATURE_COMPAT_EXCLUDE_INODE	0x0080 not used, legacy */
#define EXT2_FEATURE_COMPAT_EXCLUDE_BITMAP	0x0100
#define EXT4_FEATURE_COMPAT_SPARSE_SUPER2	0x0200


#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER	0x0001
#define EXT2_FEATURE_RO_COMPAT_LARGE_FILE	0x0002
/* #define EXT2_FEATURE_RO_COMPAT_BTREE_DIR	0x0004 not used */
#define EXT4_FEATURE_RO_COMPAT_HUGE_FILE	0x0008
#define EXT4_FEATURE_RO_COMPAT_GDT_CSUM		0x0010
#define EXT4_FEATURE_RO_COMPAT_DIR_NLINK	0x0020
#define EXT4_FEATURE_RO_COMPAT_EXTRA_ISIZE	0x0040
#define EXT4_FEATURE_RO_COMPAT_HAS_SNAPSHOT	0x0080
#define EXT4_FEATURE_RO_COMPAT_QUOTA		0x0100
#define EXT4_FEATURE_RO_COMPAT_BIGALLOC		0x0200
#define EXT4_FEATURE_RO_COMPAT_METADATA_CSUM	0x0400
#define EXT4_FEATURE_RO_COMPAT_REPLICA		0x0800

#define EXT2_FEATURE_INCOMPAT_COMPRESSION	0x0001
#define EXT2_FEATURE_INCOMPAT_FILETYPE		0x0002
#define EXT3_FEATURE_INCOMPAT_RECOVER		0x0004 /* Needs recovery */
#define EXT3_FEATURE_INCOMPAT_JOURNAL_DEV	0x0008 /* Journal device */
#define EXT2_FEATURE_INCOMPAT_META_BG		0x0010
#define EXT3_FEATURE_INCOMPAT_EXTENTS		0x0040
#define EXT4_FEATURE_INCOMPAT_64BIT		0x0080
#define EXT4_FEATURE_INCOMPAT_MMP		0x0100
#define EXT4_FEATURE_INCOMPAT_FLEX_BG		0x0200
#define EXT4_FEATURE_INCOMPAT_EA_INODE		0x0400
#define EXT4_FEATURE_INCOMPAT_DIRDATA		0x1000
/* 0x2000 was EXT4_FEATURE_INCOMPAT_BG_USE_META_CSUM but this was never used */
#define EXT4_FEATURE_INCOMPAT_LARGEDIR		0x4000 /* >2GB or 3-lvl htree */
#define EXT4_FEATURE_INCOMPAT_INLINEDATA	0x8000 /* data in inode */

#define EXT2_FEATURE_COMPAT_SUPP	0
#define EXT2_FEATURE_INCOMPAT_SUPP    (EXT2_FEATURE_INCOMPAT_FILETYPE| \
				       EXT4_FEATURE_INCOMPAT_MMP)
#define EXT2_FEATURE_RO_COMPAT_SUPP	(EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER| \
					 EXT2_FEATURE_RO_COMPAT_LARGE_FILE| \
					 EXT4_FEATURE_RO_COMPAT_DIR_NLINK| \
					 EXT2_FEATURE_RO_COMPAT_BTREE_DIR)

/*
 * Ext2 directory file types.  Only the low 3 bits are used.  The
 * other bits are reserved for now.
 */
#define EXT2_FT_UNKNOWN		0
#define EXT2_FT_REG_FILE	1
#define EXT2_FT_DIR		2
#define EXT2_FT_CHRDEV		3
#define EXT2_FT_BLKDEV		4
#define EXT2_FT_FIFO		5
#define EXT2_FT_SOCK		6
#define EXT2_FT_SYMLINK		7

#define EXT2_FT_MAX		8

/*
 * Structure of a directory entry
 */
#define EXT2_NAME_LEN 255

struct ext2_dir_entry {
	uint32	inode;			/* Inode number */
	uint16	rec_len;		/* Directory entry length */
	uint16	name_len;		/* Name length */
	char		name[EXT2_NAME_LEN];	/* File name */
};


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
