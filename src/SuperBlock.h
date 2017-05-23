#ifndef SuperBlockH
#define SuperBlockH

/*
 *  This is a wrapper contains a super block structure and can converted to/from data buffer,
 *  deal with multi bytes swap for the endian issue between data stored in the NVM and in the computers.
 */
#include "Tables.h"

class CSuperBlockWrapper
{
public:
	CSuperBlockWrapper();
  ~CSuperBlockWrapper();
  
	CSuperBlockWrapper(byte* Data);
	CSuperBlockWrapper(TSuperBlock& Source);

private:
  TSuperBlock   _SuperBlock;
};

#endif
