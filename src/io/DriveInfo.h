//---------------------------------------------------------------------------
#ifndef DriveInfoH
#define DriveInfoH
//---------------------------------------------------------------------------
#include <string>
#include "BaseTypes.h"
#include "UsbDrive.h"
#include "DriveError.h"
using namespace std;
//---------------------------------------------------------------------------
class CDriveInfo
{
public:
	CDriveInfo();
	CDriveInfo(const wstring& DriveName);

  void SetDriveName(const wstring& DriveName);

	void Get();

	wstring  GetDriveName() 						{ return _DriveName; 				}
	wstring  GetVolumnName()						{ return _VolumnName; 			}
	wstring	 GetFileSysName()					  { return _FileSysName; 			}
	ulong  	 GetSerialNumber() 					{ return _SerialNumber; 		}
	ulong		 GetMaxComponentLen()				{ return _MaxComponentLen; 	}
	ulong 	 GetFileSystemFlags()				{ return _FileSysFlags;			}

	uint64	 GetFreeBytesAvailable()		{ return _FreeBytesAvailable; }
	uint64	 GetTotalBytes()   					{ return _TotalBytes;					}
	uint64   GetTotalFreeBytes()				{ return _TotalFreeBytes;			}

	wstring  ToDriveCaption();
	bool		 IsFormatted();
	bool		 IsAvailable();
	bool 		 IsNTFS();
	bool		 IsWriteProtected();

private:
	bool GetVolumnMounted(CUsbDrive& UsbDrive);

private:

	wstring		_DriveName;
	wstring		_VolumnName;
	wstring		_FileSysName;
	ulong			_SerialNumber;
	ulong			_MaxComponentLen;
	ulong 		_FileSysFlags;

	uint64		_FreeBytesAvailable;
	uint64		_TotalBytes;
	uint64		_TotalFreeBytes;

	byte			_SpaceResult;
	byte			_VolumnResult;
	bool			_Available;
	bool			_IsNTFS;
	bool			_WriteProtected;
};
//---------------------------------------------------------------------------
#endif
