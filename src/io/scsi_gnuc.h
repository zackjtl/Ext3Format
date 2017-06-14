//---------------------------------------------------------------------------
#ifndef scsi_gnucH
#define scsi_gnucH
#ifdef	__GNUC__
//---------------------------------------------------------------------------
#include <string>
#include "BaseTypes.h"
#include "StdCbw.h"

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <scsi/sg.h>
#include <fcntl.h>
#include <memory.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/fs.h>
#include <sys/ioctl.h>

using namespace std;
//---------------------------------------------------------------------------
/*
* This is for windows IO Control only but we should defined the structure body for Linux
*/
typedef struct _OVERLAPPED {
  unsigned long* Internal;
  unsigned long* InternalHigh;
  union {
    struct {
      dword Offset;
      dword OffsetHigh;
    };
    void*  Pointer;
  };
  HANDLE    hEvent;
} OVERLAPPED, *LPOVERLAPPED;
//---------------------------------------------------------------------------
namespace my_io_base
{
  /*
    *   The device handle open that the openning behavior depands on the OS.
    *   eg. In windows, the input DeviceName could be a drive name or symbolic name;
    *   In linux, it might be named /dev/sdxx. 
    */

  HANDLE open_handle(const string& DeviceName);
  HANDLE open_handle(const wstring& DeviceName);

  int close_handle(HANDLE Handle);

  /* Initial the scsi command structure  by the user inputed CBW. 
    *  The structure will be received by the dv_io_ctl to execute a scsi command.  */
  
  /* Note that the Sense buffer is appended in the scsi passthrow directly structure for windows, 
    * so that the parameter SenseBuffer and SenseBufferLen is not needed here for windows  */    

  void init_scsi_struct(CCbwParam&   CBW,
                        byte*        DataBuff,
                        uint         TimeOutSecond,
                        void*        StructurePtr,
                        byte*        SenseBuffer = NULL,
                        uint         SenseBufferLen = 0);

  /*
  *   Wrap the IO Control function for general used in different platform.
  */

  int dev_io_ctl(
    HANDLE  Handle,
    dword   ControlCode,
    void*   InBuffer,
    dword   InBufferSize,
    void*   OutBuffer,
    dword   OutBufferSize,
    dword*  BytesReturned,
    LPOVERLAPPED  lpOverlapped = 0);
};
//---------------------------------------------------------------------------
#endif  // ifdef __GNUC__
#endif	