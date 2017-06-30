#ifndef JournalSpWrapperH
#define JournalSpWrapperH

/*
 *  This is a wrapper contains a super block structure and can converted to/from data buffer,
 *  deal with multi bytes swap for the endian issue between data stored in the NVM and in the computers.
 */
#include "e3fs_tables.h"
#include "Bulk.h"
#include <vector>

class CJournalSpWrapper
{
public:
	CJournalSpWrapper();
  ~CJournalSpWrapper();
  
	CJournalSpWrapper(byte* Data);
	CJournalSpWrapper(TJournalSuperBlock& Source);

  void ToBuffer(Bulk<byte>& Buffer);

	std::wstring ToString(uint Indent);

  TJournalSuperBlock& GetStruct();

private:
  void Invert(TJournalSuperBlock* Source, TJournalSuperBlock* Target);

private:
  TJournalSuperBlock   _JournalSp;
};

#endif
