#ifndef Ext3FsH
#define Ext3FsH

#include "Tables.h"
#include "Types.h"
#include "Ext2Params.h"
#include "BlockGroup.h"
#include "BlockManager.h"
#include <vector>

class CExt3Fs
{
public:
  CExt3Fs(uint64 TotalSectors);
  ~CExt3Fs();

  void Create();
  

private:
	void InitSuperBlock();
  void CreateBlockManager();
  void CreateBlockGroups();
  
	void CalcReservedGdtBlocks();
	void CalcInodeNumber();

private:
  

public:
  TSuperBlock   Super;
  uint64        TotalSectors;  
  uint32        InodeBlocksPerGroup;

  CExt2Params   Params;

	std::vector<CBlockGroup*> 	BlockGroups;
	auto_ptr<CBlockManager>	  	BlockMan;
  
};

#endif
