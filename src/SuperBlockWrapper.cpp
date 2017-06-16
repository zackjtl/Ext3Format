#include "SuperBlockWrapper.h"
#include "TypeConv.h"
#include "KeyValuePrinter.h"
#include <cstring>

CSuperBlockWrapper::CSuperBlockWrapper()
{
}

CSuperBlockWrapper::~CSuperBlockWrapper()
{
}

/*
 *  Convert from data buffer to the structure.
 */
CSuperBlockWrapper::CSuperBlockWrapper(byte* Data)
{
  Invert((TSuperBlock*) Data, &_SuperBlock);
}

/*
 *  Copy from a structure.
 */
CSuperBlockWrapper::CSuperBlockWrapper(TSuperBlock& Source)
{
  _SuperBlock = Source;
}

/*
 *  Convert from the structure to data buffer.
 */
void CSuperBlockWrapper::ToBuffer(Bulk<byte>& Buffer)
{
	Invert(&_SuperBlock, (TSuperBlock*) Buffer.Data());
}


/*
 *  Invert high/low bytes for I/O
 */
#define invt(x) {x = endian.Swap(x);}

void CSuperBlockWrapper::Invert(TSuperBlock* Source, TSuperBlock* Target)
{
  CEndian      endian;

  memcpy(Target, Source, sizeof(TSuperBlock));

  invt(Target->InodeCnt);
  invt(Target->BlockCnt);
  invt(Target->ReservedBlockCnt);
  invt(Target->FreeBlockCnt);
  invt(Target->FreeInodeCnt);
  invt(Target->FirstDataBlk);
  invt(Target->BlockSizeFlag);
	invt(Target->FragmentSize);
  invt(Target->BlocksPerGroup);
  invt(Target->FragmentsPerGroup);
  invt(Target->InodesPerGroup);
  invt(Target->LastMountTime);
  invt(Target->LastWriteTime);
  invt(Target->MountCnt);
  invt(Target->MaxMountCnt);      
  invt(Target->FileSysState);     
  invt(Target->BehaviorOnErr);    
  invt(Target->MinorRevLevel);    
  invt(Target->TimeOfLastCheck);  
  invt(Target->MaxTimeBetweenChecks); 
  invt(Target->OS);   
  invt(Target->RevLevel);
  invt(Target->UsrIdForRevBlks);    
  invt(Target->GroupIdForRevBlks);    
  invt(Target->FirstNonReservedInode);  
  invt(Target->InodeSize);              
  invt(Target->SPIndex);                
  invt(Target->CompFeatureFlags);       
  invt(Target->IncompFeatureFlags);     
  invt(Target->ROCompFeatureFlags);     
  invt(Target->AlgorithmUsageBmp);
  invt(Target->RsvdGdtBlocks);
  invt(Target->JournalInode);
  invt(Target->JournalDevice);
  invt(Target->LastOrphanedInode);
  invt(Target->HashSeed[4]);
  invt(Target->DefaultHashVersion);   
  invt(Target->DefaultMountOptions);  
  invt(Target->FirstMetaBlockGroup);
  invt(Target->FsCreateTime);  
  invt(Target->JournalBlock[12]);
  invt(Target->JournalIBlock);     
  invt(Target->JournalDIBlock);    
  invt(Target->JournalTIBlock);    
  invt(Target->Rsvd);
  invt(Target->JournalFileSize);  
  invt(Target->BlockCountHi);
  invt(Target->RsvdBlockCountHi);
  invt(Target->FreeBlockCountHi);   
}

/*
 *  Dump the struture fileds and values to a wide-string
 */
wstring CSuperBlockWrapper::ToString(uint Indent)
{
	wostringstream        text;
  wstring               spaces(Indent, L' ');
  CKeyValuePrinter  		printer(40);

	#define TABLE_NAME	_SuperBlock
	#define	SPACE_LEN		40

	dispdec(InodeCnt);
	dispdec(BlockCnt);
	dispdec(ReservedBlockCnt);
	dispdec(FreeBlockCnt);
	dispdec(FreeInodeCnt);
	dispdec(FirstDataBlk);
	dispdec(BlockSizeFlag);
	dispdec(FragmentSize);
	dispdec(BlocksPerGroup);
	dispdec(FragmentsPerGroup);
	dispdec(InodesPerGroup);
	dispdec(LastMountTime);
	dispdec(LastWriteTime);
	dispdec(MountCnt);
	dispdec(MaxMountCnt);
	disparr(Magic);
	dispdec(FileSysState);
	dispdec(BehaviorOnErr);
	dispdec(MinorRevLevel);
	dispdec(TimeOfLastCheck);
	dispdec(MaxTimeBetweenChecks);
	dispdec(OS);
	dispdec(RevLevel);
	dispdec(UsrIdForRevBlks);
	dispdec(GroupIdForRevBlks);
	dispdec(FirstNonReservedInode);
	dispdec(InodeSize);
	dispdec(SPIndex);
	dispdec(CompFeatureFlags);
	dispdec(IncompFeatureFlags);
	dispdec(ROCompFeatureFlags);
	disparr(UUIDOfVolume);
	dispdec(VolumeName);
	dispdec(LastMountedPath);
	dispdec(AlgorithmUsageBmp);
	dispdec(BlocksPreAlloc);
	dispdec(DirBlocksPreAlloc);
	dispdec(RsvdGdtBlocks);
	disparr(JournalUUID);
	dispdec(JournalInode);
	dispdec(JournalDevice);
	dispdec(LastOrphanedInode);
	disparr(HashSeed);
	dispdec(DefaultHashVersion);
	dispdec(DefaultMountOptions);
	dispdec(FirstMetaBlockGroup);
	dispdec(FsCreateTime);
	disparr(JournalBlock);
	dispdec(JournalIBlock);
	dispdec(JournalDIBlock);
	dispdec(JournalTIBlock);
	dispdec(Rsvd);
	dispdec(JournalFileSize);
	dispdec(BlockCountHi);
	dispdec(RsvdBlockCountHi);
	dispdec(FreeBlockCountHi);

  return text.str();

  #undef TABLE_NAME
  #undef SPACE_LEN
}

