//---------------------------------------------------------------------------
#include "Pch.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#ifndef __GNUC__
#include <Windows.h>
#include "usbdi.h"
#include "usb100.h"
#include "spti.h"
#include "NTDDSCSI.H"
#include <winioctl.h>
#include <Winternl.h>
#include "scsi_windows.h"
#else
#define  SCSI_IOCTL_DATA_IN 1
#define  SCSI_IOCTL_DATA_OUT 0
#define  CDB6GENERIC_LENGTH 6
#define  CDB10GENERIC_LENGTH 10
#define  SCSIOP_TEST_UNIT_READY 0x00
#define  SCSIOP_READ_CAPACITY 0x25
#define  SCSIOP_READ          0x28
#define  SCSIOP_WRITE         0x2A

#include "scsi_gnuc.h"

#endif

#include "UsbDrive.h"
#include "TypeConv.h"
#include "VolumeLock.h"
#include "USBError.h"
using namespace scsi_base;
//---------------------------------------------------------------------------
template <typename T> T SwapBytes(T Value);
//---------------------------------------------------------------------------
CUsbDrive::CUsbDrive()
	: _DiskHandle(INVALID_HANDLE_VALUE),
		_LockFree(false),
		_LastDiskName(L"")
{
}
//---------------------------------------------------------------------------
CUsbDrive::CUsbDrive(const wstring& DiskName, bool LockFree)
	: _DiskHandle(INVALID_HANDLE_VALUE),
		_LockFree(LockFree)
{
  Open(DiskName);
}
//---------------------------------------------------------------------------
CUsbDrive::~CUsbDrive()
{
  Close();
}
//---------------------------------------------------------------------------
void CUsbDrive::Open(const wstring& DiskName)
{
	Close();

	_LastDiskName = DiskName;
	_DiskHandle = open_handle(DiskName);

	tracer_msg(L"drive handle="  << _DiskHandle);

  if (_DiskHandle == INVALID_HANDLE_VALUE) {
		throw CError(ERROR_USB_OPEN_HANDLE, wstring(L"Drive not found, please detect again! " + DiskName));
	}
	if (!_LockFree) {
		_Locker.reset(new CVolumeLock(_DiskHandle));
	}
}
//---------------------------------------------------------------------------
void CUsbDrive::ReOpen()
{
  Open(_LastDiskName);
}
//---------------------------------------------------------------------------
bool CUsbDrive::IsOpen()
{
  return _DiskHandle != INVALID_HANDLE_VALUE ? true : false;
}
//---------------------------------------------------------------------------
void CUsbDrive::Close()
{
	if (_DiskHandle != INVALID_HANDLE_VALUE) {
		if (!_LockFree) {
			_Locker.reset();
		}
		close_handle(_DiskHandle);
    _DiskHandle = INVALID_HANDLE_VALUE;
  }
}
//---------------------------------------------------------------------------
dword CUsbDrive::ReadCapacity()
{
    BOOL                status;
    dword               returned;
    //SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER  sptdwb;
#ifndef __GNUC__
    int structSize = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
    dword ioctlCode = IOCTL_SCSI_PASS_THROUGH_DIRECT;
#else
    int structSize = sizeof(sg_io_hdr_t);
    dword ioctlCode = SG_IO;
#endif  
    byte sptdwBuffer[512]; 
    memset(sptdwBuffer, 0, 512);

  CCbwParam        CBW;

  CBW.Reset();
  CBW.SetTransLenInByte(8);
  CBW.SetFlag(SCSI_IOCTL_DATA_IN);
  CBW.SetLun(0);
  CBW.SetCbdLen(CDB10GENERIC_LENGTH);
  CBW.SetOpCode(SCSIOP_READ_CAPACITY, 0);

  byte dataBuff[512];
  byte senseBuff[32];

	init_scsi_struct(CBW, dataBuff, 2, sptdwBuffer, senseBuff, sizeof(senseBuff));

  tracer_msg(L"\r\n SCSI: Read Capacity");
  status = dev_io_ctl(_DiskHandle, ioctlCode, sptdwBuffer, structSize, sptdwBuffer, structSize, &returned, NULL);

  if( status == -1 ) {
		throw CError(L"[Error] : Cannot read USB disk size.");
  }
  tracer_msg(L"\r\n SCSI: Status: Success");

	dword capacity = SwapBytes(*reinterpret_cast<dword*>(&dataBuff[0]));

  return capacity ? capacity + 1 : 0;
}
//---------------------------------------------------------------------------
void CUsbDrive::ReadSector(dword Address, uint SectorCount, byte* OutBuf)
{
  //WaitForDiskReady(300);
  BOOL                                  status;
  dword                                 returned;
  //SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER  sptdwb;
#ifndef __GNUC__
  int structSize = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
  dword ioctlCode = IOCTL_SCSI_PASS_THROUGH_DIRECT;
#else
  int structSize = sizeof(sg_io_hdr_t);
  dword ioctlCode = SG_IO;
#endif
  byte sptdwBuffer[512];
  memset(sptdwBuffer, 0, 512);

  CCbwParam        CBW;

  CBW.Reset();
  CBW.SetTransLenInByte(SectorCount * 512);
  CBW.SetFlag(SCSI_IOCTL_DATA_IN);
  CBW.SetLun(0);
  CBW.SetCbdLen(CDB10GENERIC_LENGTH);
  CBW.SetOpCode(SCSIOP_READ, 0);

  CEndian endian;
	CBW.SetLba(endian.Swap((dword)Address));
	CBW.SetCount(endian.Swap((word)SectorCount));

  byte senseBuff[24];

	init_scsi_struct(CBW, OutBuf, 6, sptdwBuffer, senseBuff, sizeof(senseBuff));

  status = dev_io_ctl(_DiskHandle, ioctlCode, sptdwBuffer, structSize, sptdwBuffer, structSize, &returned, NULL);
  if (status == -1) {
    wostringstream text;

    text << L"[Error] : Cannot read "
         << dec << (int) SectorCount
         << L" sectors at address = 0x"
         << hex << right << setfill(L'0') << setw(8) << Address
				 << L".";
		// Don'y explain too detail - Jerry 2015.08
		throw CError(ERROR_USB_TRANSATION, L"USB Transation Error (0)");
	}
  tracer_msg(L"\r\n SCSI: Status: Success");
}
//---------------------------------------------------------------------------
void CUsbDrive::WriteSector(dword Address, uint SectorCount, byte* InBuf)
{
  BOOL                                  status;
  dword                                 returned;
  //SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER  sptdwb;

#ifndef __GNUC__
  int structSize = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
  dword ioctlCode = IOCTL_SCSI_PASS_THROUGH_DIRECT;
#else
  int structSize = sizeof(sg_io_hdr_t);
  dword ioctlCode = SG_IO;
#endif

  byte sptdwBuffer[512]; 
  memset(sptdwBuffer, 0, 512);

  CCbwParam        CBW;

  CBW.Reset();
  CBW.SetTransLenInByte(SectorCount * 512);
  CBW.SetFlag(SCSI_IOCTL_DATA_OUT);
  CBW.SetLun(0);
  CBW.SetCbdLen(CDB10GENERIC_LENGTH);
  CBW.SetOpCode(SCSIOP_WRITE, 0);
  CEndian endian;
  CBW.SetLba(endian.Swap((dword)Address));
  CBW.SetCount(endian.Swap((word)SectorCount));

  byte senseBuff[24];

  init_scsi_struct(CBW, InBuf, 6, sptdwBuffer, senseBuff, sizeof(senseBuff));
  
  status = dev_io_ctl(_DiskHandle, ioctlCode, sptdwBuffer, structSize, sptdwBuffer, structSize, &returned, NULL);
  
  if (status == -1) {
    wostringstream text;

    text << L"[Error] : Cannot write "
         << dec << (int) SectorCount
         << L" sectors at address = 0x"
         << hex << right << setfill(L'0') << setw( 8 ) << Address
				 << L".";
		// Don'y explain too detail - Jerry 2015.08
		throw CError(ERROR_USB_TRANSATION, L"USB Transation Error (1)");
  }
  tracer_msg(L"\r\n SCSI: Status: Success");
}
//---------------------------------------------------------------------------
#ifndef __GNUC__
void CUsbDrive::ReadSectorFast(dword Address, uint SectorCount, byte* OutBuff)
{
  int ret;
  unsigned long written;
  long realAddr;
  long higher = 0;
  bool higherEn = false;

  if (Address >= 0x400000) {
    realAddr = (long)(Address << 9);
    higher = (long)(Address >> 23);
    higherEn = true;
  }
  else {
    realAddr = Address * 512;
    higherEn = false;
  }

  SetFilePointer(_DiskHandle, realAddr, higherEn ? &higher : NULL, FILE_BEGIN);

  ret  = ReadFile(_DiskHandle, (LPVOID) OutBuff, (DWORD) SectorCount * 512, &written, NULL);
}
//---------------------------------------------------------------------------
void CUsbDrive::WriteSectorFast(dword Address, uint SectorCount, byte* InBuf)
{
  int ret;
  unsigned long written;
  long realAddr;
  long higher = 0;
  bool higherEn = false;

  if (Address >= 0x400000) {
    realAddr = (long)(Address << 9);
    higher = (long)(Address >> 23);
    higherEn = true;
  }
  else {
    realAddr = Address * 512;
    higherEn = false;
  }

  SetFilePointer(_DiskHandle, realAddr, higherEn ? &higher : NULL, FILE_BEGIN);

  ret  = WriteFile(_DiskHandle, (LPVOID) InBuf, (DWORD) SectorCount * 512, &written, NULL);
}
#endif
//---------------------------------------------------------------------------
void CUsbDrive::WaitForDiskReady(uint Timeout)
{
  uint                                  delay = 5;
  BOOL                                  status;
  dword                                 returned;  

#ifndef __GNUC__
  int structSize = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
  dword ioctlCode = IOCTL_SCSI_PASS_THROUGH_DIRECT;
#else
  int structSize = sizeof(sg_io_hdr_t);
  dword ioctlCode = SG_IO;
#endif

  byte sptdwBuffer[512]; 
  memset(sptdwBuffer, 0, 512);

  CCbwParam        CBW;

  CBW.Reset();
  CBW.SetTransLenInByte(0);
  CBW.SetFlag(SCSI_IOCTL_DATA_IN);
  CBW.SetLun(0);
  CBW.SetCbdLen(CDB6GENERIC_LENGTH);
  CBW.SetOpCode(SCSIOP_TEST_UNIT_READY, 0);
  CBW.SetCount(0);

  byte senseBuff[32];

  init_scsi_struct(CBW, NULL, 6, sptdwBuffer, senseBuff, sizeof(senseBuff));

  for (uint count = Timeout / delay + 1; count; --count) {
		status = dev_io_ctl(_DiskHandle, ioctlCode, sptdwBuffer, structSize, sptdwBuffer, structSize, &returned, NULL);
    
    if (status != -1) {
      if (!GetScsiStatus(sptdwBuffer)) {
        return;
      }
		}
#ifndef __GNUC__
		Sleep(delay);
#endif
  }
  throw CError(L"[Error] : Disk is not ready.");
}
//---------------------------------------------------------------------------
bool CUsbDrive::GetScsiStatus(byte* StructBuffer)
{
#ifndef __GNUC__
  SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER* sptdw = (SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER*)StructBuffer;
  return sptdw->sptd.ScsiStatus;
#else
  sg_io_hdr_t* sg_io_hdr = (sg_io_hdr_t*)StructBuffer;
  return sg_io_hdr->status;
#endif
}
//---------------------------------------------------------------------------
void CUsbDrive::ForceLockControl(bool Lock)
{
	if (_LockFree) return;

	if (Lock) {
		_Locker.reset(new CVolumeLock(_DiskHandle));
	}
	else {
    _Locker.reset();
  }
}
//---------------------------------------------------------------------------
HANDLE CUsbDrive::GetHandle()
{
	return _DiskHandle;
}
//---------------------------------------------------------------------------
bool CUsbDrive::GetLockStatus()
{
	if (_Locker.get() != NULL) {
    return _Locker->GetLockStatus();
	}
	return false;
}
//---------------------------------------------------------------------------
template <typename T>
T SwapBytes(T Value)
{
	for (unsigned int idx = 0; idx < (sizeof(T) / 2); ++idx) {
		unsigned char* src  = reinterpret_cast<unsigned char*>(&Value) + idx;
    unsigned char* dest = reinterpret_cast<unsigned char*>(&Value) + sizeof(T) - 1 - idx;
		unsigned char  temp = *dest;

		*dest = *src;
    *src = temp;
	}
	return Value;
}
//---------------------------------------------------------------------------

