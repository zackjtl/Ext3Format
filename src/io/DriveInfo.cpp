//---------------------------------------------------------------------------
#pragma hdrstop
#include "Pch.h"
#ifndef __GNUC__
#include <windows.h>
#include <WinIoCtl.h>
#endif
#include "DriveInfo.h"
#include "ErrorType.h"
#include "TypeConv.h"
#include "UsbDrive.h"
#include "DiskVolumn.h"
//---------------------------------------------------------------------------
CDriveInfo::CDriveInfo()
	:	_SpaceResult(0),
		_VolumnResult(0),
		_Available(false)
{
	_DriveName = L"";
}
//---------------------------------------------------------------------------
CDriveInfo::CDriveInfo(const wstring& DriveName)
	: _DriveName(DriveName),
		_SpaceResult(0),
		_VolumnResult(0),
		_Available(false)
{
}
//---------------------------------------------------------------------------
void CDriveInfo::SetDriveName(const wstring& DriveName)
{
  _DriveName = DriveName;
}
//---------------------------------------------------------------------------
void CDriveInfo::Get()
{
#ifdef __GNUC__
#else
	wchar_t				volumnName[256];
	wchar_t				fsName[256];
	dword 				lastError;

	_SpaceResult = 0;
	_VolumnResult = 0;
	_Available = false;

	_FreeBytesAvailable = 0;
	_TotalBytes = 0;
	_TotalFreeBytes = 0;
	_MaxComponentLen = 0;
	_SerialNumber = 0;
	_FileSysFlags = 0;

	if (_DriveName == L"") {
    return;
  }
	_VolumnResult = GetVolumeInformationW(
											_DriveName.c_str(),
											volumnName,
											256,
											&_SerialNumber,
											&_MaxComponentLen,
											&_FileSysFlags,
											fsName,
											256);

	if (_FileSysFlags & FILE_READ_ONLY_VOLUME) {
		_WriteProtected = true;
	}

	lastError = GetLastError();

	wstring fsNameW = fsName;
	bool isNTFS = false;

	if (fsNameW == L"NTFS") {
		isNTFS = true;
	}

	CUsbDrive  usbDrive(L"\\\\.\\" + _DriveName, isNTFS);

	if (lastError != 21) {
		// Drive mounted but no file system (volumn)
		_TotalBytes = (uint64)usbDrive.ReadCapacity() * 512;
		_Available = true;
	}
	bool initialLockOk = usbDrive.GetLockStatus();

	if (usbDrive.IsOpen()) {
		if (isNTFS || GetVolumnMounted(usbDrive)) {
			if (initialLockOk) {
			  usbDrive.ForceLockControl(false);
			}

			_SpaceResult = GetDiskFreeSpaceExW(_DriveName.c_str(),
																				(PULARGE_INTEGER)&_FreeBytesAvailable,
																				(PULARGE_INTEGER)&_TotalBytes,
																				(PULARGE_INTEGER)&_TotalFreeBytes);

			lastError = GetLastError();

			if (initialLockOk) {
			  usbDrive.ForceLockControl(true);
			}

			if (_SpaceResult != 0) {
				_Available = true;
			}
			else if (lastError != 21) {
				// Drive mounted but no file system (volumn)
				_TotalBytes = usbDrive.ReadCapacity() * 512;
				_Available = true;
			}
    }
	}
	if (initialLockOk) {
	  usbDrive.ForceLockControl(false);
	}

	if (_VolumnResult != 0) {
		_VolumnName = volumnName;
		_FileSysName = fsName;
	}
	else {
		_VolumnName = L"";
		_FileSysName = L"No FS";
	}
#endif
}
//---------------------------------------------------------------------------
bool CDriveInfo::IsNTFS()
{
	if (_FileSysName == L"NTFS") {
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
bool CDriveInfo::GetVolumnMounted(CUsbDrive& UsbDrive)
{
#ifdef __GNUC__
	return true;
#else
	if (!UsbDrive.IsOpen()) {
		return false;
	}
	HANDLE diskHandle = UsbDrive.GetHandle();

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
wstring CDriveInfo::ToDriveCaption()
{
	uint bytesMB = (_TotalBytes / 1024) / 1024;
	wstring text;

	if (_SpaceResult != 0 && _VolumnResult != 0) {
		text = 	_DriveName + L"  " +
						IntegerToWideString(bytesMB) + L" MB [" +
						_FileSysName + L"] ";
	}
	else {
		text = 	_DriveName + L"  " +
						IntegerToWideString(bytesMB) + L" MB  [" +
						_FileSysName + L"] ";
  }

	return text;
}
//---------------------------------------------------------------------------
bool CDriveInfo::IsFormatted()
{
	if ((_VolumnResult != 0) && (_SpaceResult != 0)) {
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
bool CDriveInfo::IsAvailable()
{
  return _Available;
}
//---------------------------------------------------------------------------
bool CDriveInfo::IsWriteProtected()
{
 	return _WriteProtected;
}
//---------------------------------------------------------------------------
#pragma package(smart_init)
