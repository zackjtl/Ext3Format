//---------------------------------------------------------------------------
#ifndef Ext3fsckH
#define Ext3fsckH
//---------------------------------------------------------------------------
#include "Ext3Fs.h"
#include "Tester.h"
#include "UsbDrive.h"
#include "FolderInodeReader.h"
//---------------------------------------------------------------------------
class CExt3Fsck : public CExt3Fs
{
public:

  CExt3Fsck(CUsbDrive& Drive);
  ~CExt3Fsck();
  void LoadFs();

private:
  void ReadSuperBlock();
  void ReadAndInitBlockGroups();
  void ReadBlockBmp();
  void ReadInodeBmp();

  void RebuildRootDir();
  void RebuildFolder(CFolderInodeReader* Dir);
  void GetOrReadInodeTable(uint32 InodeId, TInode& Inode);

  void ReadInodeTableBlocks();
  void ReadGroupInodeTable(uint32 Group);  

  void ReadBlocksToBuffer(uint32 Block, uint32 BlockCount, Bulk<byte>& Buffer);

  void RebuildJournalInode();
  
private:
  CUsbDrive&    _Drive;
  bool          _BlockBmpLoaded;
};
//---------------------------------------------------------------------------
#endif
