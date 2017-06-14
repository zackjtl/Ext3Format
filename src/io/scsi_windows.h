//---------------------------------------------------------------------------
#ifndef scsi_windowsH
#define scsi_windowsH
#ifndef __GNUC__
//---------------------------------------------------------------------------
#include <string>
#include "BaseTypes.h"

#ifdef UT33X_USED
	#include "Ut33xCbw.h"
	#include "UT33xDevice.h"
#endif

#include "StdCbw.h"

#include <windows.h>
#include <string>
#include <memory>
#include <limits>          // It can check boundarie
#include "BaseError.h"
#include "usbdi.h"
#include "usb100.h"
////#include "usb2sd.h"
#include <sstream>
#include <iomanip>

using namespace std;
//---------------------------------------------------------------------------
namespace scsi_base
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
  #ifdef UT33X_USED
  void init_ut33x_scsi_struct(CUt33xCbw&    CBW,
			                        byte*        DataBuff,
			                        uint         TimeOutSecond,
			                        void*        StructurePtr,
			                        byte*        SenseBuffer = NULL,   
															uint         SenseBufferLen = 0);
	#endif

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
#endif // ifndef __GNUC__
#endif	

