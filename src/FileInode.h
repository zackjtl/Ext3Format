//---------------------------------------------------------------------------
#ifndef FileInodeH
#define FileInodeH
//---------------------------------------------------------------------------
#include "Inode.h"
#include "BlockManager.h"

class CFileInode : public CInode
{
public:
  CFileInode(uint32 BlockSize);
  ~CFileInode();



  /* TODO:   
  void ReadData(byte* Buffer, uint32 Length);
  void Seek(uint64 Position);
  */


private:

};

#endif
