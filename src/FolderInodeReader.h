//---------------------------------------------------------------------------
#ifndef FolderInodeReaderH
#define FolderInodeReaderH
//---------------------------------------------------------------------------
#include "InodeReader.h"

class CFolderInodeReader : public CInodeReader
{
public:
  CFolderInodeReader(TInode* InodeIn, uint16 BlockSize);
  ~CFolderInodeReader();

  void Read(CBlockManager& BlockMan, CUsbDrive* Drive);

  void AddEntry(ext2_dir_entry& Entry);

  uint32    DotInode;
  uint32    TwoDotInode;  
  

private:
  std::map<std::string, uint32>   _NameList; 
  
};

#endif
