#ifndef __GNUC__
//---------------------------------------------------------------------------
#include "Pch.h"
#pragma hdrstop

#include "scsi_windows.h"
#include "TypeConv.h"
#include "USBError.h"
#include "NTDDSCSI.H"
//---------------------------------------------------------------------------
HANDLE scsi_base::open_handle(const string& DeviceName)
{
  HANDLE handle;

  handle = CreateFileA(DeviceName.c_str(),
                      GENERIC_READ | GENERIC_WRITE,
                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                      NULL,
                      OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL,
                      NULL);
  return handle;
}
//---------------------------------------------------------------------------
HANDLE scsi_base::open_handle(const wstring& DeviceName)
{
  return open_handle(ToNarrowString(DeviceName));
}
//---------------------------------------------------------------------------
int scsi_base::close_handle(HANDLE Handle)
{
  return CloseHandle(Handle);
}
//---------------------------------------------------------------------------
int scsi_base::dev_io_ctl(
  HANDLE  Handle,
  dword   ControlCode,
  void*   InBuffer,
  dword   InBufferSize,
  void*   OutBuffer,
  dword   OutBufferSize,
  dword*  BytesReturned,
  LPOVERLAPPED  lpOverlapped)
{
  int status;

  status = DeviceIoControl(Handle,
                           ControlCode,
                           InBuffer,
                           InBufferSize,
                           OutBuffer,
                           OutBufferSize,
                           BytesReturned,
                           lpOverlapped);

  if (status == 0) {
    status = -1;
  }
  return status;
}
//---------------------------------------------------------------------------
#ifdef UT33X_USED
void scsi_base::init_ut33x_scsi_struct(CUt33xCbw    CBW,
			                                  byte*        DataBuff,
			                                  uint         TimeOutSecond,
			                                  void*        StructurePtr,
			                                  byte*        SenseBuffer,
			                                  uint         SenseBufferLen)
{
  SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER* sptdwb = (SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER*)StructurePtr;

  ZeroMemory(StructurePtr, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));

  sptdwb->sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
  sptdwb->sptd.PathId = 0;
  sptdwb->sptd.TargetId = 1;
  sptdwb->sptd.Lun = 0;
  sptdwb->sptd.CdbLength = CBW.GetCbdLen(); //CDB10GENERIC_LENGTH;
  sptdwb->sptd.SenseInfoLength = 24;

  if( !CBW.GetFlag() ) {
    sptdwb->sptd.DataIn = SCSI_IOCTL_DATA_OUT;
  }
  else {
    sptdwb->sptd.DataIn = SCSI_IOCTL_DATA_IN;
  }

  sptdwb->sptd.DataTransferLength = CBW.GetTransLenInByte(); // big-endia.

  if (TimeOutSecond!=0) {
    sptdwb->sptd.TimeOutValue = TimeOutSecond;
  }
  else {
    sptdwb->sptd.TimeOutValue = 10;
  }

  sptdwb->sptd.DataBuffer = DataBuff;
  sptdwb->sptd.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, ucSenseBuf);

  CBW.GetCbd(&sptdwb->sptd.Cdb[0],CBW.GetCbdLen());

  sptdwb->sptd.ScsiStatus=0xff;
}
#endif
//---------------------------------------------------------------------------
void scsi_base::init_scsi_struct(CCbwParam&   				CBW,
                                  byte*               DataBuff,
                                  uint                TimeOutSecond,
                                  void*               StructurePtr,
                                  byte*               SenseBuffer,
                                  uint                SenseBufferLen)
{
  SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER* sptdwb = (SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER*)StructurePtr;

  ZeroMemory(StructurePtr, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));

  sptdwb->sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
  sptdwb->sptd.PathId = 0;
  sptdwb->sptd.TargetId = 1;
  sptdwb->sptd.Lun = 0;
  sptdwb->sptd.CdbLength = CBW.GetCbdLen(); //CDB10GENERIC_LENGTH;
  sptdwb->sptd.SenseInfoLength = 24;

  if( !CBW.GetFlag() ) {
    sptdwb->sptd.DataIn = SCSI_IOCTL_DATA_OUT;
  }
  else {
    sptdwb->sptd.DataIn = SCSI_IOCTL_DATA_IN;
  }

  sptdwb->sptd.DataTransferLength = CBW.GetTransLenInByte(); // big-endia.

  if (TimeOutSecond!=0) {
    sptdwb->sptd.TimeOutValue = TimeOutSecond;
  }
  else {
    sptdwb->sptd.TimeOutValue = 10;
  }

  sptdwb->sptd.DataBuffer = DataBuff;
  sptdwb->sptd.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, ucSenseBuf);

  CBW.GetCbd(&sptdwb->sptd.Cdb[0],CBW.GetCbdLen());

  sptdwb->sptd.ScsiStatus=0xff;
}
//---------------------------------------------------------------------------
#endif // ifndef __GNUC__
