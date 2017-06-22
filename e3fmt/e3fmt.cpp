#pragma hdrstop
#pragma argsused

#ifdef _WIN32
#include <tchar.h>
#else
  typedef char _TCHAR;
  #define _tmain main
#endif

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "Ext3Fs.h"
#include "UsbDrive.h"
#include "BaseError.h"
#include "TypeConv.h"
#include "BaseTypes.h"

using namespace std;

enum fmtt {fmtt_ext2, fmtt_ext3};

byte      FmtType = fmtt_ext3;
string    DeviceName = "";
bool      HelpMode = false;

void PRS(int argc, _TCHAR* argv[]);
void PrintCapacity(uint32 Capacity);

 int _tmain(int argc, _TCHAR* argv[])
{
  PRS(argc, argv);

  if (DeviceName == "") {
    printf("Not yet indicated device name, please input the device name:\n");
    cin >> DeviceName;
    DeviceName = "\\\\.\\" + DeviceName + ":";
  }
  printf("Try to open handle of device %s\n", DeviceName.c_str());

  wstring devName = ToWideString(DeviceName);
  unique_ptr<CUsbDrive> drive;
  unique_ptr<CExt3Fs> fs;
  uint32 capacity;

  try {
    drive.reset((new CUsbDrive(devName)));
  }
  catch (CError& Error) {
    printf("[Error] Unable to open the device handle, got error with message..%s\n ",
            ToNarrowString(Error.GetText()));
    goto PROG_END;
  }

  if (drive->IsOpen() == false) {
    printf("[Error] Unable to open the device handle\n");
    goto PROG_END;
  }

  capacity = drive->ReadCapacity();

  PrintCapacity(capacity);
  printf("Create extended file system..\n");

  /* Create file system with the device capacity */
  fs.reset(new CExt3Fs(capacity));
  fs->Create();
  printf("Creation done. Write system into device..\n");
  fs->Write(*drive.get());
  printf("Done\n");

PROG_END:
  system("PAUSE");
	return 0;
}

/*
 *  parse input arguments
 */
void PRS(int argc, _TCHAR* argv[])
{
  for (int i = 1; i < argc; ) {
    if (argv[i][0] == L'-') {
      if ((strcmp(argv[i], "-help") == 0) ||
          (strcmp(argv[i], "-h") == 0)) {
        HelpMode = true;
        return;
      }

      if (strcmp(argv[i], "-t") == 0) {
        ++i;

        if (strcmp(argv[i], "ext2") == 0) {
          FmtType = fmtt_ext2;
        }
        else if (strcmp(argv[i], "ext3") == 0) {
          FmtType = fmtt_ext3;
        }
        else {
          throw CError(L"Unrecognized indicated format");
        }
      }
    }
    else {
      string device = argv[i];

      #ifdef __GNUC__
      if (device.find("//dev//") == 0) {
        /* A linux usb device */
        DeviceName = device;
      }
      #else
      if (device[1] == L':') {
        /* A windows usb device*/
        DeviceName = "////.//" + device;
      }
      #endif
      else {
        //throw CError(L"Unknown input argument");
        DeviceName = "";
      }
    }
    ++i;
  }

  printf("Format Type:   %s\n", FmtType == fmtt_ext2 ? "ext2" : "ext3");
  printf("Target Device: %s\n", DeviceName.c_str());
}

/*
 *  print capacity
 */
void PrintCapacity(uint32 Capacity)
{
	double totalGB = ((double)Capacity / 2097152);
	double totalMB = ((double)Capacity / 512);
	double totalKB = ((double)Capacity / 2);

  printf("Device capacity: %d sectors ");

	if (totalGB >= 1) {
		printf("(%.2f GB)\n", totalGB);
	}
	else if (totalMB >= 10) {
		printf("(%d MB)\n", (int)totalMB);
	}
	else if (totalKB >= 10) {
		printf("(%.2f KB)\n", totalKB);
	}
	else {
		printf("(%d Bytes)\n", Capacity);
	}
}
