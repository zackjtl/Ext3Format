#include <vcl.h>
#include <windows.h>

#pragma hdrstop
#pragma argsused

#include <tchar.h>
#include <stdio.h>
#include <vector>
#include <string>

#include <sstream>
#include <iomanip>
#include <iostream>

#include "BaseError.h"
#include "Tester.h"
#include "TestBlockManager.h"
#include "TestInode.h"
#include "TestInodeWrite.h"
#include "TestExt3Fs.h"
////#include "my_uuid.h"
#include "TestUUID.h"

using namespace std;

void show_caption()
{
}

int _tmain(int argc, _TCHAR* argv[])
{
	static const test_case tests[] =
								 {{show_caption, L"Block Manager Test"},
									{TestAutoAllocBlock, L"Auto Alloc Block"},
									{TestAutoAllocSingleBlock, L"Auto Alloc Single Block"},
									{TestOccupyBlock, L"Occupy Block"},
									{TestSetBlockData, L"Set Block Data"},
									{TestGetBlockData, L"Get Block Data"},
									{TestSetAndGetSingleBlockData, L"Set/Get Single Block Data"},
									{TestBlockOverwrite, L"Block Overwrite"},
									{show_caption, L"Utility Test"},
									{Test_uuid_v4_generator, L"Test UUID V4 Generator"},
									{show_caption, L"Inode Alloc Block Test"},
									{TestAllocDirBlocks, L"Alloc Direct Blocks"},
									{TestAllocIndirBlocks, L"Alloc Indirect Blocks"},
									{TestAllocDIndirBlocks, L"Alloc Double-Indirect Blocks"},
									{TestAllocTIndirBlocks, L"Alloc Tripple-Indirect Blocks"},
									{show_caption, L"Inode Data Access Test"},
									{WriteDirectBlockToIndirectBlock, L"Write inode from direct to indirect blocks"},
									{WriteIndirectBlockToDIndirectBlock, L"Write inode from indirect to d_indirect blocks"},
									{WriteDIndirectBlockToTIndirectBlock, L"Write inode from d_indirect to t_indirect blocks"},
									{ValidateIndirectTable, L"Validate address table linking of indirect blocks"},
									{ValidateDIndirectTable, L"Validate address table linking of d_indirect blocks"},
									{ValidateTIndirectTable, L"Validate address table linking of t_indirect blocks"},
									{show_caption, L"EXT3 File System Creation"},
									{TestExt3Fs128GB, L"EXT3 File System with 128GB"},
									{TestExt3FsReadWrite, L"Test real format storage with ext3"}};


	int cnt = sizeof(tests) / sizeof(tests[0]);
	////vector<test_case> arr(ts, ts + sizeof(ts) / sizeof(ts[0]));

	for (int i = 0; i < cnt; ++i) {
		if (tests[i].fn == show_caption) {
			wprintf(L"-------- %s --------\n", tests[i].name.c_str());
			continue;
		}

		try {
			tests[i].fn();
			wprintf(L"[OK] %s\n", tests[i].name.c_str());
		}
		catch (CError& Error) {
			wprintf(L"\n[FAIL] %s\n [%08x]: %s\n", tests[i].name.c_str(),
					Error.GetCode(), Error.GetText().c_str());
		}
		catch (...) {
			wprintf(L"\n[FAIL] %s\n [FFFE]: Unkown\n", tests[i].name.c_str());
		}
	}

	/*
	byte uuid[16];

	gen_uuid_v4(uuid, 16);

	for (int i = 0; i < 4; ++i) {
		wcout << hex << setw(2) << setfill(L'0') << uuid[i];
	}
	wcout << L"-";
	for (int i = 4; i < 6; ++i) {
		wcout << hex << setw(2) << setfill(L'0') << uuid[i];
	}
	wcout << L"-";
	for (int i = 6; i < 8; ++i) {
		wcout << hex << setw(2) << setfill(L'0') << uuid[i];
	}
	wcout << L"-";
	for (int i = 8; i < 10; ++i) {
		wcout << hex << setw(2) << setfill(L'0') << uuid[i];
	}
	wcout << L"-";
	for (int i = 10; i < 16; ++i) {
		wcout << hex << setw(2) << setfill(L'0') << uuid[i];
	} */

	system("PAUSE");
	return 0;
}
