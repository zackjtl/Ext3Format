//---------------------------------------------------------------------------
#ifndef TestInodeWriteH
#define TestInodeWriteH
//---------------------------------------------------------------------------
#include "BaseTypes.h"

void WriteDirectBlockToIndirectBlock();
void WriteIndirectBlockToDIndirectBlock();
void WriteDIndirectBlockToTIndirectBlock();

void ValidateIndirectTable();
void ValidateDIndirectTable();
void ValidateTIndirectTable();


/* This is not a test function */
void make_pattern(byte* Buffer, byte StartPattern, uint32 Length);

#endif
