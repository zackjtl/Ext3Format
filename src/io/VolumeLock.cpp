//---------------------------------------------------------------------------
#include "Pch.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#ifndef __GNUC__
#include <Windows.h>
#include <WinIoCtl.h>
#include "scsi_windows.h"
#endif
#include "BaseError.h"
#include "VolumeLock.h"
#include "USBError.h"

#ifdef __GNUC__
#define FSCTL_LOCK_VOLUME   LOCK_EX
#define FSCTL_UNLOCK_VOLUME LOCK_UN

#include "scsi_gnuc.h"
#endif

using namespace scsi_base;

//---------------------------------------------------------------------------
CVolumeLock::CVolumeLock(HANDLE Handle)
	: _Handle(Handle),
		_LockStatus(false)
{
	Lock();
}
//---------------------------------------------------------------------------
CVolumeLock::~CVolumeLock()
{
  Unlock();
}
//---------------------------------------------------------------------------
void CVolumeLock::Lock()
{
  LockUnLockVolume(FSCTL_LOCK_VOLUME);
}
//---------------------------------------------------------------------------
void CVolumeLock::Unlock()
{
  LockUnLockVolume(FSCTL_UNLOCK_VOLUME);
}
//---------------------------------------------------------------------------
bool CVolumeLock::GetLockStatus()
{
	return _LockStatus;
}
//---------------------------------------------------------------------------
bool CVolumeLock::IsWindows7OrLater()
{
#ifdef __GNUC__
  return false;
#else
  ::OSVERSIONINFO osvi;
  bool IsWindowsVistaPorLater;

  ::ZeroMemory(&osvi, sizeof(OSVERSIONINFO));

  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

  GetVersionEx(&osvi);

  IsWindowsVistaPorLater =
     ( (osvi.dwMajorVersion > 6) ||
     ( (osvi.dwMajorVersion == 6) && (osvi.dwMinorVersion >= 1) ));

  if (IsWindowsVistaPorLater) {
    return true;
  }
  else {
    return false;
  }
#endif
}
//---------------------------------------------------------------------------
void CVolumeLock::LockUnLockVolume(dword ControlCode)
{
#ifdef __GNUC__
  if (flock(_Handle, ControlCode) == 0) {
    //printf(" Lock Operation Success \r\n");
  }
  else {
    //printf(" Lock Operation Fail \r\n");
  }
#else
	dword         bytesReadWrite;
	const  uint   TRY_COUNT = 3;

  if (IsWindows7OrLater()){
    for (uint idx =0; idx < TRY_COUNT; ++idx) {
			_LockStatus = DeviceIoControl(_Handle,
																		ControlCode, NULL,
																		0,
																		NULL,
																		0,
																		&bytesReadWrite,
																 		NULL);

		 if (_LockStatus) {
      break;
     }
     else if (idx == TRY_COUNT) {
      if (ControlCode == FSCTL_LOCK_VOLUME){
        throw CError(ERROR_USB_READER_LOST, L"[Lock]: Cannot open USB device.");
      }
      else {
        throw CError(ERROR_USB_READER_LOST, L"[UnLock]: Cannot open USB device.");
      }
    }
    else{
      Sleep(250);
    }
   }
  }
#endif
}
//---------------------------------------------------------------------------

