#ifndef SuperBlockWrapperH
#define SuperBlockWrapperH

/*
 *  This is a wrapper contains a super block structure and can converted to/from data buffer,
 *  deal with multi bytes swap for the endian issue between data stored in the NVM and in the computers.
 */
#include "Tables.h"
#include "Bulk.h"
#include <string>

class CSuperBlockWrapper
{
public:
	CSuperBlockWrapper();
  ~CSuperBlockWrapper();
  
	CSuperBlockWrapper(byte* Data);
	CSuperBlockWrapper(TSuperBlock& Source);

	void ToBuffer(Bulk<byte>& Buffer);
	std::wstring ToString(uint Indent);

private:
  void Invert(TSuperBlock* Source, TSuperBlock* Target);

private:
  TSuperBlock   _SuperBlock;
};

#endif
