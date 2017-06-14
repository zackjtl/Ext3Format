#ifndef USBErrorH
#define USBErrorH
//----------------------------------------------------------------------------
#include "ErrorType.h"
//----------------------------------------------------------------------------
enum {
	ERROR_USB_BULK_IN = ERROR_USB_PORT << 24, ERROR_USB_READER_LOST,
	ERROR_USB_TRANSATION, ERROR_USB_BULK_OUT, ERROR_USB_SEND_CBW,
	ERROR_USB_READ_CSW, ERROR_USB_GET_PIPE_INFO,
	ERROR_ABORT_USB_PIPE, ERROR_RESET_USB_PORT, ERROR_GET_LAST_USB_ERROR, ERROR_USB_OPEN_HANDLE
};
//----------------------------------------------------------------------------
#endif
