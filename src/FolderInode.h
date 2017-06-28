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
	CFolderInode(uint16 BlockSize);
	~CFolderInode();

	void Attach(CInode* Inode, CBlockManager& BlockMan, TSuperBlock& Super);

	/* TODO:
	void RebuildInodeBlocks(byte* Buffer, uint32 Length);
	void Seek(uint64 Position);
	*/
  
	void MkDir(uint32 Dot, uint32 TwoDot,
			       CBlockManager& BlockMan, TSuperBlock& Super);

  void UpdateInodeTable();

  uint32    DotInode;
  uint32    TwoDotInode;

private:
	void MakeEntry(ext2_dir_entry& Entry, uint32 Inode, uint8 FileType,
									const char* Name, TSuperBlock& Super);

  void AddEntry(ext2_dir_entry& Entry, CBlockManager& BlockMan,
                TSuperBlock& Super);        

protected:
	std::map<std::string, uint32>   _NameList; 
  bool                            _Initialized;  

  uint32    _LastEntryOffset;
  uint32    _LastEntryBlock;
};

#endif
