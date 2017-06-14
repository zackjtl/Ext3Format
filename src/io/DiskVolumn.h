//---------------------------------------------------------------------------
#ifndef DiskVolumnH
#define DiskVolumnH
//---------------------------------------------------------------------------
#include "UsbDrive.h"
#include <string>
using namespace std;
//---------------------------------------------------------------------------
class CDiskVolumn
{
public:
	CDiskVolumn(const wstring& DriveName);

  void Refresh();

	bool IsDriveOpen();
	bool IsVolumnMounted();
	CUsbDrive& GetUsbDrive();

	uint64	 GetFreeBytesAvailable()		{ return _FreeBytesAvailable; }
	uint64	 GetTotalBytes()   					{ return _TotalBytes;					}
	uint64   GetTotalFreeBytes()				{ return _TotalFreeBytes;			}

private:
	bool		 GetVolumnMounted();
	bool		 GetFreeSpaceSuccessed();

private:
	uint64		_FreeBytesAvailable;
	uint64		_TotalBytes;
	uint64		_TotalFreeBytes;

	bool			_IsVolumnMounted;
	bool			_GetFreeSpaceSuccessed;

private:
	wstring			_DriveName;
	CUsbDrive 	_UsbDrive;
};
//---------------------------------------------------------------------------
#endif
