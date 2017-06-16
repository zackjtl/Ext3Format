#ifdef __GNUC__
//---------------------------------------------------------------------------
#include "Pch.h"
#pragma hdrstop

#include "scsi_gnuc.h"
#include "TypeConv.h"
#include "USBError.h"
//---------------------------------------------------------------------------
HANDLE scsi_base::open_handle(const string& DeviceName)
{
  HANDLE handle;
  handle = open(DeviceName.c_str(), O_RDWR);
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
  close(Handle);
  return 1;
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
  status = ioctl(Handle, ControlCode, InBuffer);
  return status;
}
//---------------------------------------------------------------------------
void scsi_base::init_scsi_struct(CCbwParam&   CBW,
                                  byte*        DataBuff,
                                  uint         TimeOutSecond,
                                  void*        StructurePtr,
                                  byte*        SenseBuffer,
                                  uint         SenseBufferLen)
{
  sg_io_hdr_t* io_hdr = (sg_io_hdr_t*)StructurePtr;

  memset((byte*)io_hdr, 0, sizeof(sg_io_hdr_t));
  io_hdr->interface_id    = 'S';
  io_hdr->cmd_len         = CBW.GetCbdLen();
  io_hdr->dxfer_direction = CBW.GetFlag()? SG_DXFER_FROM_DEV : SG_DXFER_TO_DEV;
  io_hdr->dxfer_len       = CBW.GetTransLenInByte();
  io_hdr->dxferp          = DataBuff;
  io_hdr->cmdp            = CBW.GetCbd();
  io_hdr->mx_sb_len       = SenseBufferLen;
  io_hdr->sbp             = SenseBuffer;
  io_hdr->timeout         = TimeOutSecond;     /* 1000 millisecs == 1 seconds */
}
//---------------------------------------------------------------------------
#endif	// ifdef __GNUC__
