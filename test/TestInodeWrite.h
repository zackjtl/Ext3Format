//---------------------------------------------------------------------------
#ifndef TestInodeWriteH
#define TestInodeWriteH
//---------------------------------------------------------------------------
#include "Types.h"

void WriteDirectBlockToIndirectBlock();
void WriteIndirectBlockToDIndirectBlock();
void WriteDIndirectBlockToTIndirectBlock();


/* This is not a test function */
void make_pattern(byte* Buffer, byte StartPattern, uint32 Length);

#endif
