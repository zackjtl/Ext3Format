#include "JournalSpWrapper.h"
#include "TypeConv.h"
#include "KeyValuePrinter.h"
#include <string>

using namespace std;

CJournalSpWrapper::CJournalSpWrapper()
{
}

CJournalSpWrapper::~CJournalSpWrapper()
{
}

/*
 *  Convert from data buffer to the structure.
 */
CJournalSpWrapper::CJournalSpWrapper(byte* Data)
{
  Invert((TJournalSuperBlock*) Data, &_JournalSp);
}

/*
 *  Copy from a structure.
 */
CJournalSpWrapper::CJournalSpWrapper(TJournalSuperBlock& Source)
{
  _JournalSp = Source;
}

/*
 *  Convert from the structure to data buffer.
 */
void CJournalSpWrapper::ToBuffer(Bulk<byte>& Buffer)
{
	Invert(&_JournalSp, (TJournalSuperBlock*) Buffer.Data());
}

TJournalSuperBlock& CJournalSpWrapper::GetStruct()
{
  return _JournalSp;
}

/*
 *  Invert high/low bytes for I/O
 */
#define invt(x) {x = endian.Swap(x);}

void CJournalSpWrapper::Invert(TJournalSuperBlock* Source, TJournalSuperBlock* Target)
{
  CEndian      endian;

  memcpy(Target, Source, sizeof(TJournalSuperBlock));  

  invt(Target->Header.Signature);
  invt(Target->Header.BlockType);
  invt(Target->Header.SequenceNumber);
 
  invt(Target->BlockSize);
  invt(Target->BlockCount);
  invt(Target->StartBlock);
  invt(Target->SequenceOf1stTrans);
  invt(Target->BlockOf1stTrans);
  
  invt(Target->Error);
  invt(Target->CompFeatureFlags);
  invt(Target->IncompFeatureFlags);
  invt(Target->ROCompFeatureFlags);

  invt(Target->Users);
  invt(Target->BlocksOfDynamicSBCopy);
  invt(Target->MaxTransaction);
  invt(Target->MaxTransactionBlocks);

  endian.Swap((dword*)Target->Padding, 42);
  invt(Target->Checksum);
}
/*
 *  Dump the struture fileds and values to a wide-string
 */
wstring CJournalSpWrapper::ToString(uint Indent)
{
	wostringstream        text;
  wstring               spaces(Indent, L' ');
  CKeyValuePrinter  		printer(40);

	#define TABLE_NAME	_JournalSp
	#define	SPACE_LEN		40

	dispdec(Header.Signature);
	dispdec(Header.BlockType);
	dispdec(Header.SequenceNumber);
	dispdec(BlockSize);
	dispdec(BlockCount);
	dispdec(StartBlock);
	dispdec(SequenceOf1stTrans);
	dispdec(BlockOf1stTrans);
  
  dispdec(Error);
  dispdec(CompFeatureFlags);
  dispdec(IncompFeatureFlags);
  dispdec(ROCompFeatureFlags);

  disparr(UUID);

  dispdec(Users);
  dispdec(BlocksOfDynamicSBCopy);
  dispdec(MaxTransaction);
  dispdec(MaxTransactionBlocks);

  dispdec(ChecksumType);

  disparr(Padding2);
  disparr(Padding);

  dispdec(Checksum);
  disparr(User_ID);  

  return text.str();
}

