//---------------------------------------------------------------------------
#ifndef KeyValuePrinterH
#define KeyValuePrinterH
//---------------------------------------------------------------------------
#include <string>
#include "BaseTypes.h"
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
	wstring print(wstring Name, string Value);
	wstring print(wstring Name, byte* Value, uint Length, uint Indent=0, bool LineNo = false);
	wstring print(wstring Name, word* Value, uint Length, uint Indent=0, bool LineNo = false);
  wstring print(wstring Name, dword* Value, uint Length, uint Indent=0);

private:
	const uint _key_length;
	bool	_IniFormat;
};
//---------------------------------------------------------------------------
/*
 *	The macros for general used in a table printer.
 *  1. Need to re-define TABLE_NAME int every using function.
 *  2. Need to declare wostringstream text and wstring spaces in the using funciton.
 *  3. Need to declare a CKeyValuePrinter object if want to printf arrays or
 *		 chip-blocks.
 *	4. Need to re-define SPACE_LEN for structures that have a longer field names.
 */

#define _CAT(x, y) x##y
#define _SIZEOF(x) (sizeof(_FIELD(x)))
#define _LENOF(x) (_SIZEOF(x) / sizeof(_FIELD(x)[0]))
#define _FIELD(x) (TABLE_NAME.##x)
#ifndef endl
	#define endl L"\r\n"
#endif
#define SPACE_LEN 30

#define disphex(x) { text << spaces << setw(SPACE_LEN) << setfill(L' ') << left << #x \
													<< L"= 0x" << setw(_SIZEOF(x)*2) << setfill(L'0') << right << hex \
													<< _FIELD(x) << endl; }

#define dispdec(x) { text << spaces << setw(SPACE_LEN) << setfill(L' ') << left << #x \
													<< L"= " << _FIELD(x) << endl; }

#define disparr(x) { text << spaces << printer.print(_CAT(L, #x), _FIELD(x), _LENOF(x), Indent); }

#define disp2darr(x, i) { text << spaces << printer.print((wstring)_CAT(L, #x)\
													 + L"[" + IntegerToWideString(i) + L"]", _FIELD(x[i]), _LENOF(x[i]), Indent); }

#define splitline(c, len)      { text << spaces << setw(len) << setfill(c) << L"" << endl;}

#define disp_chip_block(x, idx) { text << spaces << printer.print((wstring)_CAT(L, #x)\
																	 + L"[" + IntegerToWideString(idx) + L"]", _FIELD(x[idx])) << endl; }

#define disp_a_chip_block(x) { text << spaces << printer.print((wstring)_CAT(L, #x), \
															_FIELD(x)) << endl; }

#define disp_chip_blocks(x, cnt) {for (int i = 0; i < cnt; ++i) {disp_chip_block(x, i); };}
//---------------------------------------------------------------------------
#endif
