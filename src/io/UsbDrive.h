//---------------------------------------------------------------------------
#ifndef UsbDriveH
#define UsbDriveH
//---------------------------------------------------------------------------
#include <string>
#include "BaseTypes.h"
#include "VolumeLock.h"
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
template <typename T> T SwapBytes(T Value);
//---------------------------------------------------------------------------
class CUsbDrive
{
public:
	CUsbDrive();
	CUsbDrive(const wstring& DiskName, bool LockFree = false);
	~CUsbDrive();

	void  Open(const wstring& DiskName);
	void  ReOpen();
  bool  IsOpen();
	void  Close();
	dword ReadCapacity();
  void  ReadSector(dword Address, uint SectorCount, byte* OutBuf);
	void  WriteSector(dword Address, uint SectorCount, byte* InBuf);
  #ifndef __GNUC__
  void  ReadSectorFast(dword Address, uint SectorCount, byte* OutBuf);
	void  WriteSectorFast(dword Address, uint SectorCount, byte* InBuf);
  #endif
	void  WaitForDiskReady(uint Timeout);
	HANDLE GetHandle();
	bool  GetLockStatus();

	void ForceLockControl(bool Lock);

private:
  bool GetScsiStatus(byte* StructBuffer);

private:
	static const uint MAX_SECTOR_COUNT = 128;
private:
	HANDLE   	_DiskHandle;
	unique_ptr<CVolumeLock>  _Locker;
	wstring		_LastDiskName;
	bool			_LockFree;
};
//---------------------------------------------------------------------------
#endif
