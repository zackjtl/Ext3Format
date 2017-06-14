//---------------------------------------------------------------------------
#ifndef ReaderInfoH
#define ReaderInfoH
#include "BaseTypes.h"
#include "SdDef.h"
#include <string>

using namespace std;
//---------------------------------------------------------------------------
#ifdef CUSTOM_UTILITY
enum {
  RT_NORMAL_READER, RT_UT33X_READER, RT_UT33X_NATIVE_READER
};
#else
enum {
	RT_USB2SD_READER = 0, RT_UT33X_READER, RT_NORMAL_READER
};
#endif
//---------------------------------------------------------------------------
class CReaderInfo
{
public:
  CReaderInfo()
  {
    _BusPower = PWR3_3V;
  }
  CReaderInfo(const CReaderInfo& Source)
    : _Type(Source._Type),
      _SerialNum(Source._SerialNum),
      _DeviceName(Source._DeviceName),
      _BusPower(Source._BusPower)
  {
  }
  CReaderInfo& operator=(const CReaderInfo& Source) 
  {
    if (&Source != this) {
      _Type       = Source._Type;
      _SerialNum  = Source._SerialNum;
      _DeviceName = Source._DeviceName;
      _BusPower   = Source._BusPower;
    }
    return *this;
  }
  void SetType(uint Value)                  { _Type       = Value; }
  void SetSerialNum(uint Value)             { _SerialNum  = Value; }
  void SetDeviceName(const wstring& Value)  { _DeviceName = Value; }
  void SetBusPower(byte Value)              { _BusPower   = Value; }

  uint     GetType()             const { return _Type;       }
  uint     GetSerialNum()        const { return _SerialNum;  }
  const wstring& GetDeviceName() const { return _DeviceName; }
  byte     GetBusPower()         const { return _BusPower;   }

private:
  uint      _Type;
  uint      _SerialNum;
  wstring   _DeviceName;
  byte      _BusPower;
};
//---------------------------------------------------------------------------
#endif
