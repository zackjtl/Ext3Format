#ifndef InodeWrapperH
#define InodeWrapperH

/*
 *  This is a wrapper contains a super block structure and can converted to/from data buffer,
 *  deal with multi bytes swap for the endian issue between data stored in the NVM and in the computers.
 */
#include "e3fs_tables.h"
#include "Bulk.h"
#include <string>

class CInodeWrapper
{
public:
	CInodeWrapper();
  ~CInodeWrapper();
  
	CInodeWrapper(byte* Data);
	CInodeWrapper(TInode& Source);

	void ToBuffer(Bulk<byte>& Buffer);
	std::wstring ToString(uint Indent);

private:
  void Invert(TInode* Source, TInode* Target);

private:
	TInode   _Inode;
};

#endif
