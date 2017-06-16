#include "GroupDescWrapper.h"
#include "TypeConv.h"
#include "KeyValuePrinter.h"
#include <cstring>

CGroupDescWrapper::CGroupDescWrapper()
{
}

CGroupDescWrapper::~CGroupDescWrapper()
{
}

/*
 *  Convert from data buffer to the structure.
 */
CGroupDescWrapper::CGroupDescWrapper(byte* Data)
{
  Invert((TGroupDesc*) Data, &_GroupDesc);
}

/*
 *  Copy from a structure.
 */
CGroupDescWrapper::CGroupDescWrapper(TGroupDesc& Source)
{
  _GroupDesc = Source;
}

/*
 *  Convert from the structure to data buffer.
 */
void CGroupDescWrapper::ToBuffer(Bulk<byte>& Buffer)
{
	Invert(&_GroupDesc, (TGroupDesc*) Buffer.Data());
}


/*
 *  Invert high/low bytes for I/O
 */
#define invt(x) {x = endian.Swap(x);}

void CGroupDescWrapper::Invert(TGroupDesc* Source, TGroupDesc* Target)
{
  CEndian      endian;

  memcpy(Target, Source, sizeof(TGroupDesc));  

  invt(Target->BlockBmpBlock);
  invt(Target->InodeBmpBlock);
  invt(Target->InodeTableBlock);
 
  invt(Target->SnapshotExcludeBmp);
}

/*
 *  Dump the struture fileds and values to a wide-string
 */
wstring CGroupDescWrapper::ToString(uint Indent)
{
	wostringstream        text;
  wstring               spaces(Indent, L' ');

	#define TABLE_NAME	_GroupDesc
	#define	SPACE_LEN		30

	dispdec(BlockBmpBlock);
	dispdec(InodeBmpBlock);
	dispdec(InodeTableBlock);
	dispdec(FreeBlockCount);
	dispdec(FreeInodeCount);
	dispdec(DirectoriesCount);
	disphex(Flags);
	disphex(SnapshotExcludeBmp);
	dispdec(BlockBmpChecksum);
	dispdec(InodeBmpChecksum);
	dispdec(NeverUsedInodeCnt);
	dispdec(GroupDescChecksum);

  return text.str();

  #undef TABLE_NAME
  #undef SPACE_LEN
}

