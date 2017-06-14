//---------------------------------------------------------------------------
#ifndef VolumeLockH
#define VolumeLockH
//---------------------------------------------------------------------------
#include "Pch.h"
#include "BaseTypes.h"
//---------------------------------------------------------------------------
class CVolumeLock
{
public:
	CVolumeLock(HANDLE Handle);
	~CVolumeLock();

	void Lock();
	void Unlock();

  bool GetLockStatus();

private:
  bool IsWindows7OrLater();
	void LockUnLockVolume(dword ControlCode);

private:
	HANDLE   _Handle;
	bool		 _LockStatus;
};
//---------------------------------------------------------------------------
#endif
