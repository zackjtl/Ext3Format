//---------------------------------------------------------------------------
#ifndef FolderInodeH
#define FolderInodeH
//---------------------------------------------------------------------------
#include "Inode.h"
#include "BlockManager.h"
#include <vector>

class CFolderInode : public CInode
{
public:
	CFolderInode(uint32 BlockSize);
	~CFolderInode();

	void Attach(CInode* Inode, CBlockManager& BlockMan, TSuperBlock& Super);

	/* TODO:
	void ReadData(byte* Buffer, uint32 Length);
	void Seek(uint64 Position);
	*/
  
	void MkDir(uint32 Dot, uint32 TwoDot,
			       CBlockManager& BlockMan, TSuperBlock& Super);

  void UpdateInodeTable();

  uint32    _DotInode;
  uint32    _TwoDotInode;

private:
	void MakeEntry(ext2_dir_entry& Entry, uint32 Inode, uint16 Mode,
									const char* Name, TSuperBlock& Super);

protected:
	std::map<std::string, uint32>   _NameList; 
  bool                            _Initialized;  
};

#endif
