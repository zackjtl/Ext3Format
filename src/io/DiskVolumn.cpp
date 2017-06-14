//---------------------------------------------------------------------------
#include "Pch.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#ifndef __GNUC__
#include <windows.h>
#include <winioctl.h>
#endif
#include "DiskVolumn.h"
//---------------------------------------------------------------------------
CDiskVolumn::CDiskVolumn(const wstring& DriveName)
	: _DriveName(DriveName),
		_UsbDrive(DriveName)
{
}
//---------------------------------------------------------------------------
void CDiskVolumn::Refresh()
{
	_IsVolumnMounted = GetVolumnMounted();
#ifndef __GNUC__
	_GetFreeSpaceSuccessed = GetDiskFreeSpaceEx(_DriveName.c_str(),
																		(PULARGE_INTEGER)&_FreeBytesAvailable,
																		(PULARGE_INTEGER)&_TotalBytes,
																		(PULARGE_INTEGER)&_TotalFreeBytes);
#endif
}
//---------------------------------------------------------------------------
bool CDiskVolumn::IsDriveOpen()
{
	return _UsbDrive.IsOpen();
}
//---------------------------------------------------------------------------
bool CDiskVolumn::GetVolumnMounted()
{
#ifdef __GNUC__
	return true;
#else
	if (!_UsbDrive.IsOpen()) {
		return false;
	}
	HANDLE diskHandle = _UsbDrive.GetHandle();

  dword bytesReturned;

	BOOL result = DeviceIoControl(
										diskHandle,
										FSCTL_IS_VOLUME_MOUNTED,
										NULL,
										0,
										NULL,
										0,
										&bytesReturned,
										NULL
										);

	dword lastError = GetLastError();

	if (result) {
		return true;
	}
	return false;
#endif
}
//---------------------------------------------------------------------------
bool CDiskVolumn::IsVolumnMounted()
{
	return _IsVolumnMounted;
}
//---------------------------------------------------------------------------
CUsbDrive& CDiskVolumn::GetUsbDrive()
{
  return _UsbDrive;
}
//---------------------------------------------------------------------------
