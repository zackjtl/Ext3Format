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
  /* Read inode data from media device */
  uint32 ReadDirectBlocks(CBlockManager& BlockMan, CUsbDrive* Drive, byte* Buffer, uint32 Length);
  uint32 ReadIndirectLeafBlocks(CBlockManager& BlockMan, CUsbDrive* Drive, byte* Buffer, uint32 Length);
  uint32 ReadMultiIndirectLeafBlocks(CBlockManager& BlockMan, CUsbDrive* Drive,
                                    uint32 Layer, byte* Buffer, uint32 Length);

private:

  void ReadInodeData(CBlockManager& BlockMan, CUsbDrive* Drive);

private:
  std::map<std::string, uint32>   _NameList; 
  
};

#endif
