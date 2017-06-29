//---------------------------------------------------------------------------
#ifndef JournalInodeReaderH
#define JournalInodeReaderH
//---------------------------------------------------------------------------
#include "InodeReader.h"
#include "Tables.h"

class CJournalInodeReader : public CInodeReader
{
public:
  CJournalInodeReader(TInode* InodeIn, uint16 BlockSize);
  ~CJournalInodeReader();

  void CheckJournalSuperBlock();
  void Read(CBlockManager& BlockMan, CUsbDrive* Drive);

private:
  TJournalSuperBlock  _JournalSp;

};

#endif
