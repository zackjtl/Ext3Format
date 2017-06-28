#ifndef Ext3FsH
#define Ext3FsH

#include "Tables.h"
#include "BaseTypes.h"
#include "Ext2Params.h"
#include "BlockGroup.h"
#include "BlockManager.h"
#include "FolderInode.h"
#include <vector>
#include "UsbDrive.h"

class CExt3Fs
{
public:
  CExt3Fs();
  CExt3Fs(uint64 TotalSectors);
  ~CExt3Fs();

  void Create();

	void Write(CUsbDrive& Drive);

protected:
	void InitSuperBlock();
  void CreateBlockManager();
  void CreateBlockGroups();

	void CalcReservedBlocks();
	void CalcReservedGdtBlocks();
	void CalcInodeNumber();

  void CreateRootDirectory();
	void CreateResizeInode();
  void CreateJournalInode();
	void CreateLostAndFoundDirectory();
  void CreateSpecialInodes();
  
  void UpdateBlockGroupsInfo();
  void UpdateInodeTables();
  void UpdateSuperBlock();

	void FlushGroupDescData();
	void FlushBlockBmp();
	void FlushInodeBmp();
  void FlushInodeTables();
  void FlushSuperBlock();

	void WriteArea(CUsbDrive& Drive, uint32 StartBlock, uint32 Count);

public:
  TSuperBlock   Super;
  uint64        TotalSectors;  
  uint32        InodeBlocksPerGroup;

  CExt2Params   Params;

	std::vector<CBlockGroup*> 		BlockGroups;
	unique_ptr<CBlockManager>	   	BlockMan;

	CFolderInode*               	RootInode;
};

#endif
