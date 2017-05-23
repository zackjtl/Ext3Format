//---------------------------------------------------------------------------
#ifndef KeyValuePrinterH
#define KeyValuePrinterH
//---------------------------------------------------------------------------
#include <string>
#include "Types.h"
using namespace std;
//---------------------------------------------------------------------------
class CKeyValuePrinter
{
public:
	CKeyValuePrinter()
		: _key_length(30)
	{
    _IniFormat = false;
	}
	CKeyValuePrinter(uint KeyLength, bool IniFormat = false)
		: _key_length(KeyLength),
			_IniFormat(IniFormat)
	{
	}

public:
	wstring print(wstring Name, byte Value);
	wstring print(wstring Name, word Value);
	wstring print(wstring Name, dword Value);
	wstring printdec(wstring Name, byte Value);
	wstring printdec(wstring Name, word Value);
	wstring printdec(wstring Name, dword Value);
	wstring print(wstring Name, wstring Value);
	wstring print(wstring Name, byte* Value, uint Length, uint Indent=0, bool LineNo = false);
	wstring print(wstring Name, word* Value, uint Length, uint Indent=0, bool LineNo = false);
  wstring print(wstring Name, dword* Value, uint Length, uint Indent=0);

private:
	const uint _key_length;
	bool	_IniFormat;
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#endif
