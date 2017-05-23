//---------------------------------------------------------------------------
#include "Pch.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "KeyValuePrinter.h"
#include "TypeConv.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
wstring CKeyValuePrinter::print(wstring Name, byte Value)
{
	uint gap = _key_length - Name.length() - 1;

	wstring spaces(gap, L' ');
	wstring text = Name + spaces + L" = " + L"0x" + IntegerToHexWideString(Value, 2);

	if (_IniFormat) {
    text += L";";
  }

	return text.c_str();
}
//---------------------------------------------------------------------------
wstring CKeyValuePrinter::print(wstring Name, word Value)
{
	uint gap = _key_length - Name.length() - 1;

	wstring spaces(gap, L' ');
	wstring text = Name + spaces + L" = " + L"0x" + IntegerToHexWideString(Value, 4);

	if (_IniFormat) {
		text += L";";
	}

	return text.c_str();
}
//---------------------------------------------------------------------------
wstring CKeyValuePrinter::print(wstring Name, dword Value)
{
	uint gap = _key_length - Name.length() - 1;

	wstring spaces(gap, L' ');
	wstring text = Name+ spaces + L" = " + L"0x" + IntegerToHexWideString(Value, 8);

	if (_IniFormat) {
		text += L";";
	}

	return text.c_str();
}
//---------------------------------------------------------------------------
wstring CKeyValuePrinter::print(wstring Name, wstring Value)
{
	uint gap = _key_length - Name.length() - 1;

	wstring spaces(gap, L' ');
	wstring text = Name + spaces + L" = " + Value;

	if (_IniFormat) {
		text += L";";
	}

	return text.c_str();
}
//---------------------------------------------------------------------------
wstring CKeyValuePrinter::printdec(wstring Name, byte Value)
{
	uint gap = _key_length - Name.length() - 1;

	wstring spaces(gap, L' ');
	wstring text = Name+ spaces + L" = " + IntegerToWideString(Value);

	if (_IniFormat) {
		text += L";";
	}

	return text.c_str();
}
//---------------------------------------------------------------------------
wstring CKeyValuePrinter::printdec(wstring Name, word Value)
{
	uint gap = _key_length - Name.length() - 1;

	wstring spaces(gap, L' ');
	wstring text = Name + spaces + L" = " + IntegerToWideString(Value);

	if (_IniFormat) {
		text += L";";
	}

	return text.c_str();
}
//---------------------------------------------------------------------------
wstring CKeyValuePrinter::printdec(wstring Name, dword Value)
{
	uint gap = _key_length - Name.length() - 1;

	wstring spaces(gap, L' ');
	wstring text = Name + spaces + L" = " + IntegerToWideString(Value);

	if (_IniFormat) {
		text += L";";
	}

	return text.c_str();
}
//---------------------------------------------------------------------------
wstring CKeyValuePrinter::print(wstring Name, byte* Value, uint Length, uint Indent, bool LineNo)
{
	uint gap = _key_length - Name.length();
	wstring gap_space(gap, L' ');
  
	wstring key = Name + gap_space + L"= ";

  uint firstLineIndent = key.length() + Indent;
  wstring shift_spaces(firstLineIndent, L' ');

  if (Name == L"") {
		key = gap_space;
	}

	bool firstLine = true;
  
	if (LineNo) {
		if (Name == L"") {
      key += L"   ";
    }
		key +=                L"       00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\r\n";
		key += shift_spaces + L"------------------------------------------------------\r\n";
		firstLine = false;
	}
	uint remain = Length;
	uint offset = 0;
	while (remain) {
		if (!firstLine) {			
			key += shift_spaces;
		}
		else {
      firstLine = false;
    }
		uint lineLen = remain < 16 ? remain : 16;

		if (LineNo) {
			key += IntegerToHexWideString(offset, 4) + L" : ";
		}
		for (uint idx = 0; idx < lineLen; ++idx) {
			key += IntegerToHexWideString(Value[offset + idx], 2) + L" ";
		}
		offset += lineLen;
		remain -= lineLen;

		if (remain == 0 && _IniFormat) {
			key += L";";
		}
		key += L"\r\n";
	}
	return key.c_str();
}
//---------------------------------------------------------------------------
wstring CKeyValuePrinter::print(wstring Name, word* Value, uint Length, uint Indent, bool LineNo)
{
	uint gap = _key_length - Name.length();
	wstring spaces(gap, L' ');
	wstring key = Name + spaces + L"= ";

  uint firstLineIndent = key.length() + Indent;
  wstring shift_spaces(firstLineIndent, L' ');

	bool firstLine = true;
	if (LineNo) {
		key +=                L"        00   01   02   03   04   05   06   07\r\n";
		key += shift_spaces + L"----------------------------------------------\r\n";
		firstLine = false;
	}

	uint remain = Length;
	uint offset = 0;
	while (remain) {
		if (!firstLine) {
			key += shift_spaces;
		}
		else {
      firstLine = false;
    }
		uint lineLen = remain < 8 ? remain : 8;

		if (LineNo) {
			key += IntegerToHexWideString(offset, 4) + L" : ";
    }
		for (uint idx = 0; idx < lineLen; ++idx) {
			key += IntegerToHexWideString(Value[offset + idx], 4) + L" ";
		}
		offset += lineLen;
		remain -= lineLen;

		if (remain == 0 && _IniFormat) {
			key += L";";
		}
		key += L"\r\n";
	}
	//CDataBuf  dataBuf(16, 2, DE_LITTLE_ENDIAN, false, Indent + _key_length);

	//key += dataBuf.Dump((byte*)&Value[0], Length * 2, true);

	if (_IniFormat) {
		key += L";";
	}

	return key.c_str();
}
//---------------------------------------------------------------------------
wstring CKeyValuePrinter::print(wstring Name, dword* Value, uint Length, uint Indent)
{
	uint gap = _key_length - Name.length() - 1;
	wstring spaces(gap, L' ');
	wstring key = Name + spaces + L" = ";

	CDataBuf  dataBuf(16, 4, DE_LITTLE_ENDIAN, false, Indent + _key_length + 2);

	key += dataBuf.Dump((byte*)&Value[0], Length * 4, true);

	if (_IniFormat) {
		key[key.length()-1] = L';';
		key += L"\r\n";
	}

	return key.c_str();
}
//---------------------------------------------------------------------------
