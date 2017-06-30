#ifndef GroupDescWrapperH
#define GroupDescWrapperH

/*
 *  This is a wrapper contains a super block structure and can converted to/from data buffer,
 *  deal with multi bytes swap for the endian issue between data stored in the NVM and in the computers.
 */
#include "e3fs_tables.h"
#include "Bulk.h"
#include <vector>

class CGroupDescWrapper
{
public:
	CGroupDescWrapper();
  ~CGroupDescWrapper();
  
	CGroupDescWrapper(byte* Data);
	CGroupDescWrapper(TGroupDesc& Source);

  void ToBuffer(Bulk<byte>& Buffer);

	std::wstring ToString(uint Indent);

private:
  void Invert(TGroupDesc* Source, TGroupDesc* Target);

private:
  TGroupDesc   _GroupDesc;
};

#endif
