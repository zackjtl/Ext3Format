#ifndef e3fs_uuidH
#define e3fs_uuidH

#include "BaseTypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

namespace e3fs_uuid {
	static uint16 swap_int16(uint16 val)
	{
			return (val << 8) | ((val >> 8) & 0xFF);
	}
}

static void gen_uuid_v4(byte* uuid, uint32 BufferLen)
{
  if (BufferLen < 16) {
    return;
  }

	srand(time(NULL));

	uint16* ptr16 = (uint16*)&uuid[0];

	ptr16[0] = rand();
	ptr16[1] = rand();
	ptr16[2] = rand();
	ptr16[3] = ((((uint16)rand()) & 0x0fff) | 0x4000);
	ptr16[4] = ((uint16)rand()) % 0x3fff + 0x8000;

	ptr16[3] = e3fs_uuid::swap_int16(ptr16[3]);
	ptr16[4] = e3fs_uuid::swap_int16(ptr16[4]);

	for (int i = 10; i < 16; ++i) {
  	uuid[i] = rand();
	}
}


#endif
