//---------------------------------------------------------------------------
#pragma hdrstop
#include "TestExt3Fs.h"
#include "Tester.h"
#include "UsbDrive.h"
//---------------------------------------------------------------------------
void TestExt3Fs128GB()
{
	CExt3Fs* fs = new CExt3Fs(241827840);

	fs->Create();

	TSuperBlock& super = fs->Super;
	CExt2Params& params = fs->Params;

	assert_eq(params.GroupCount, 923);
	assert_eq(super.InodeCnt, 7561216);
	assert_eq(super.InodesPerGroup, 8192);
	assert_eq(super.RsvdGdtBlocks, 1016);
}

wstring driveName = L"\\\\.\\D:";
unique_ptr<CUsbDrive> drive;

//---------------------------------------------------------------------------
void TestExt3FsReadWrite()
{
  drive.reset(new CUsbDrive(driveName));
  
	if (!drive->IsOpen()) {
		throw CError(L"Open drive " + driveName + L" failed");
	}

	uint32 capacity = drive->ReadCapacity();

	CExt3Fs* fs = new CExt3Fs(capacity);
	fs->Create();
	fs->Write(*drive.get());
}
