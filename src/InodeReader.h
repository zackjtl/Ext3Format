//---------------------------------------------------------------------------
#ifndef InodeReaderH
#define InodeReaderH
//---------------------------------------------------------------------------
#include "Inode.h"
#include "UsbDrive.h"
//---------------------------------------------------------------------------
class CInodeReader : public CInode
{
public:

  static CInodeReader* Create(TInode* Inode, uint16 BlockSize);

  CInodeReader(TInode* Inode, uint16 BlockSize);
  ~CInodeReader();

  bool IsFolderInode();
  virtual void Read(CBlockManager& BlockMan, CUsbDrive* Drive);

  Bulk<byte>& GetDataBuffer();

  /* Rebuild inode from media device */
  void RebuildInodeBlocks(CBlockManager& BlockMan, CUsbDrive* Drive);
  uint32 RebuildIndirectBlocks(CBlockManager& BlockMan, CUsbDrive* Drive, uint32 RemainBlocks);
  uint32 RebuildMultiIndirectBlocks(CBlockManager& BlockMan, CUsbDrive* Drive,
                                uint32 Layer, uint32 RemainBlocks);

protected:
  void AddBlockArray(vector<uint32>* Array, Bulk<byte>* Buffer);
  Bulk<byte>* LoadSingleBlockDataFromMedia(CBlockManager& BlockMan, CUsbDrive* Drive, uint32 Block);

protected:
  Bulk<byte>  _DataBuffer;
};
//---------------------------------------------------------------------------
#endif
