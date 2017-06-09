#include "InodeWrapper.h"
#include "TypeConv.h"
#include "KeyValuePrinter.h"

CInodeWrapper::CInodeWrapper()
{
}

CInodeWrapper::~CInodeWrapper()
{
}

/*
 *  Convert from data buffer to the structure.
 */
CInodeWrapper::CInodeWrapper(byte* Data)
{
  Invert((TInode*) Data, &_Inode);
}

/*
 *  Copy from a structure.
 */
CInodeWrapper::CInodeWrapper(TInode& Source)
{
  _Inode = Source;
}

/*
 *  Convert from the structure to data buffer.
 */
void CInodeWrapper::ToBuffer(Bulk<byte>& Buffer)
{
	Invert(&_Inode, (TInode*) Buffer.Data());
}

/*
 *  Invert high/low bytes for I/O
 */
#define invt(x) {x = endian.Swap(x);}
#define invtarr(x) {endian.Swap(x, sizeof(x) / sizeof(x[0])); }

void CInodeWrapper::Invert(TInode* Source, TInode* Target)
{
  CEndian      endian;

	memcpy(Target, Source, sizeof(TInode));

	invt(Target->Mode);
	invt(Target->Uid);
	invt(Target->SizeInBytesLo);
	invt(Target->AccessTime);
	invt(Target->InodeChangeTime);
	invt(Target->ModificationTime);
	invt(Target->DeleteTime);
	invt(Target->GroupId);
	invt(Target->HardLinkCnt);
	invt(Target->SectorCount);
	invt(Target->FileFlags);
	invt(Target->OS_Dep1);
	invtarr(Target->Blocks);
	invt(Target->FileVersion);
	invt(Target->FileACL);
	invt(Target->SizeInBytesHi);
	invt(Target->FragAddress);
	invt(Target->Padding);
	invtarr(Target->Rsvd);
}

/*
 *  Dump the struture fileds and values to a wide-string
 */
wstring CInodeWrapper::ToString(uint Indent)
{
	wostringstream        text;
  wstring               spaces(Indent, L' ');
  CKeyValuePrinter  		printer(30);

	#define TABLE_NAME	_Inode
	#define	SPACE_LEN		30

	dispdec(Mode);
	disphex(Uid);
	disphex(SizeInBytesLo);
	dispdec(AccessTime);
	dispdec(InodeChangeTime);
	dispdec(ModificationTime);
	dispdec(DeleteTime);
	disphex(GroupId);
	dispdec(HardLinkCnt);
	dispdec(SectorCount);
	disphex(FileFlags);
	disphex(OS_Dep1);
	disparr(Blocks);
	dispdec(FileVersion);
	dispdec(FileACL);
	disphex(SizeInBytesHi);
	dispdec(FragAddress);
	dispdec(FragNumber);
	dispdec(FragSize);
	disphex(Padding);
	disparr(Rsvd);

  return text.str();
}

