#ifndef TypeConvH
#define TypeConvH
//-----------------------------------------------------------------------------
////#include "Pch.h"
#include "BaseTypes.h"
#include "BaseError.h"
#include <string>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <time.h>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;
//-----------------------------------------------------------------------------
int     StringToInteger(const string& Text) throw (CError);
int     StringToInteger(const wstring& Text) throw (CError);
int64   StringToInteger64(const string& Text) throw (CError);
int64   StringToInteger64(const wstring& Text) throw (CError);
int     HexStringToInteger(const string& Text) throw (CError);
int     HexStringToInteger(const wstring& Text) throw (CError);
int64   HexStringToInteger64(const string& Text) throw (CError);
int64   HexStringToInteger64(const wstring& Text) throw (CError);
string  IntegerToString(int Value);
wstring IntegerToWideString(int Value, uint FieldWidth);
wstring IntegerToWideString(int Value);
string  Integer64ToString(int64 Value);
wstring Integer64ToWideString(int64 Value);
string  IntegerToString(int Value, uint FieldWitdh);
string  IntegerToHexString(uint Value, uint Digits);
wstring IntegerToHexWideString(uint Value, uint Digits);
string  Integer64ToHexString(int64 Value, uint Digits);
wstring Integer64ToHexWideString(int64 Value, uint Digits);
wstring DecimalStringToDolloarForm(const wstring& Text);
uint    OctalStringToInteger(const string& Text);
uint    OctalStringToInteger(const wstring& Text);
string IntegerToOctalString(uint Value, uint Digits);
wstring IntegerToOctalWideString(uint Value, uint Digits);

uint    OctToDec(uint Oct);
uint    DecToOct(uint Dec);

std::time_t GetPosixTime();

wstring GetDateTimeString();

void   SplitString(const string&    Source,
                   const string&    PartitionChar,
                   vector<string>&  OutBuf);
void   SplitString(const wstring&    Source,
                   const wstring&    PartitionChar,
                   vector<wstring>&  OutBuf);
wstring ByteArrayToHexStr(byte* InBuf, uint Size);

bool   IsDigit(const string& Text);
bool   IsDigit(const wstring& Text);
bool   IsHexDigit(const string& Text);
bool   IsHexDigit(const wstring& Text);
bool   IsAsciiString(const wstring& Text);

wstring EraseLineEnd(const wstring& Text);
wstring ElapsedTimeMsToString(uint Ms);
void CreateFolder(const wstring& Dir);
wstring ExtractMainFileName(const wstring& FileName);
//-----------------------------------------------------------------------------
enum { DE_LITTLE_ENDIAN = 0, DE_BIG_ENDIAN };
class CDataBuf
{
public:
       CDataBuf(uint Columns = 16,
                uint FieldBytes = 1,
                uint Endian = DE_BIG_ENDIAN,
                bool AsciiCharEn = true,
                uint IndentSpaces = 1)
         : _Columns(Columns),
           _FieldBytes(FieldBytes),
           _Endian(Endian),
           _AsciiCharEn(AsciiCharEn),
           _IndentSpaces(IndentSpaces)
       {
       }
  void SetColumns(uint Value)      { _Columns = Value;      }
  void SetEndian(uint Value)       { _Endian = Value;       }
  void SetFieldBytes(uint Value)   { _FieldBytes = Value;   }
  void SetIndentSpaces(uint Value) { _IndentSpaces = Value; }
  void EnableAsciiChar(bool Value) { _AsciiCharEn = Value;  }

  wstring ToText(byte* InBuf, uint Length);
  wstring ToAscii(const byte* InBuf, uint Length);
  wstring Dump(byte *InBuf, uint Length, bool SkipFirstIndent = false);
  wstring Dump(uint Address, uint Length, byte *Buffer);

private :
  uint   _Columns;
  uint   _IndentSpaces;
  uint   _FieldBytes;
  bool   _AsciiCharEn;
  uint   _Endian;
};
//-----------------------------------------------------------------------------
template <class T>
void HexStrToInts(const string& Text, vector<T>& Integers) throw (CError)
{
  Integers.clear();

  bool              stop = false;
  string::size_type start = Text.find_first_not_of(" \t", 0);

  while (start != string::npos) {
    string::size_type end = Text.find_first_of(" \t,", start + 1);
    if (end == string::npos) {
      stop = true;
      end = Text.size();
    }
    Integers.push_back(
      HexStringToInteger(Text.substr(start, end - start)));
    if (stop) {
      break;
    }
    start = Text.find_first_not_of(", \t\n", end);
  }
}
//-----------------------------------------------------------------------------
template <class T>
void HexStrToInts(const wstring& Text, vector<T>& Integers) throw (CError)
{
  Integers.clear();

  bool               stop = false;
  wstring::size_type start = Text.find_first_not_of(L" \t", 0);

  while (start != wstring::npos) {
    wstring::size_type end = Text.find_first_of(L" \t,", start + 1);
    if (end == wstring::npos) {
      stop = true;
      end = Text.size();
    }
    Integers.push_back(
      HexStringToInteger(Text.substr(start, end - start)));
    if (stop) {
      break;
    }
    start = Text.find_first_not_of(L", \t\n", end);
  }
}
//-----------------------------------------------------------------------------
template <class T>
void StrToInts(const string& Text, vector<T>& Integers) throw (CError)
{
  Integers.clear();

  bool              stop = false;
  string::size_type start = Text.find_first_not_of(" \t", 0);

  while (start != string::npos) {
    string::size_type end = Text.find_first_of(" \t,", start + 1);

    if (end == string::npos) {
      stop = true;
      end = Text.size();
    }
    Integers.push_back(StringToInteger(Text.substr(start, end - start)));

    if (stop) {
      break;
    }
    start = Text.find_first_not_of(", \t\n", end);
  }
}
//-----------------------------------------------------------------------------
template <class T>
void StrToInts(const wstring& Text, vector<T>& Integers) throw (CError)
{
  Integers.clear();

  bool               stop = false;
  wstring::size_type start = Text.find_first_not_of(L" \t", 0);

  while (start != wstring::npos) {
    wstring::size_type end = Text.find_first_of(L" \t,", start + 1);

    if (end == wstring::npos) {
      stop = true;
      end = Text.size();
    }
    Integers.push_back(StringToInteger(Text.substr(start, end - start)));

    if (stop) {
      break;
    }
    start = Text.find_first_not_of(L", \t\n", end);
  }
}
//-----------------------------------------------------------------------------
template <class T>
void ColonStrToInts(const string& Text, vector<T>& Items)
{
  Items.clear();

  string::size_type  token = Text.find(':', 0);

  if (token != string::npos) {
    string::size_type start = Text.find_first_not_of(" \t", 0);
    string::size_type end = Text.find_first_of(": \t,", start + 1);

    if (end == string::npos) {
      return;
    }
    uint begin = StringToInteger(Text.substr(start, end - start));

    start = Text.find_first_not_of(" \t", token + 1);

    if (start == string::npos) {
      return;
    }
    end = Text.find_first_of(" \t,", start + 1);

    if (end == string::npos) {
      end = Text.size();
    }
    uint final = StringToInteger(Text.substr(start, end - start));

    if (final >= begin) {
      uint idx = 0;

      Items.resize(final - begin + 1);

      for (; begin <= final; ++begin) {
        Items[idx++] = begin;
      }
    }
  }
  else {
    if (!Text.empty()) {
      Items.push_back(StringToInteger(Text));
    }
  }
}
//-----------------------------------------------------------------------------
template <class T>
void ColonStrToInts(const wstring& Text, vector<T>& Items)
{
  Items.clear();

  wstring::size_type  token = Text.find(L':', 0);

  if (token != wstring::npos) {
    wstring::size_type start = Text.find_first_not_of(L" \t", 0);
    wstring::size_type end = Text.find_first_of(L": \t,", start + 1);

    if (end == wstring::npos) {
      return;
    }
    uint begin = StringToInteger(Text.substr(start, end - start));

    start = Text.find_first_not_of(L" \t", token + 1);

    if (start == wstring::npos) {
      return;
    }
    end = Text.find_first_of(L" \t,", start + 1);

    if (end == wstring::npos) {
      end = Text.size();
    }
    uint final = StringToInteger(Text.substr(start, end - start));

    if (final >= begin) {
      uint idx = 0;

      Items.resize(final - begin + 1);

      for (; begin <= final; ++begin) {
        Items[idx++] = begin;
      }
    }
  }
  else {
    if (!Text.empty()) {
      Items.push_back(StringToInteger(Text));
    }
  }
}
//-----------------------------------------------------------------------------
template <class T>
void ColonHexStrToInts(const string& Text, vector<T>& Items)
{
  Items.clear();

  string::size_type  token = Text.find(':', 0);

  if (token != string::npos) {
    string::size_type start = Text.find_first_not_of(" \t", 0);
    string::size_type end = Text.find_first_of(": \t,", start + 1);

    if (end == string::npos) {
      return;
    }
    uint begin = HexStringToInteger(Text.substr(start, end - start));

    start = Text.find_first_not_of(" \t", token + 1);

    if (start == string::npos) {
      return;
    }
    end = Text.find_first_of(" \t,", start + 1);

    if (end == string::npos) {
      end = Text.size();
    }
    uint final = HexStringToInteger(Text.substr(start, end - start));

    if (final >= begin) {
      uint idx = 0;

      Items.resize(final - begin + 1);

      for (; begin <= final; ++begin) {
        Items[idx++] = begin;
      }
    }
  }
  else {
    if (!Text.empty()) {
      Items.push_back(HexStringToInteger(Text));
    }
  }
}
//-----------------------------------------------------------------------------
template <class T>
void ColonHexStrToInts(const wstring& Text, vector<T>& Items)
{
  Items.clear();

  wstring::size_type  token = Text.find(L':', 0);

  if (token != wstring::npos) {
    wstring::size_type start = Text.find_first_not_of(L" \t", 0);
    wstring::size_type end = Text.find_first_of(L": \t,", start + 1);

    if (end == wstring::npos) {
      return;
    }
    uint begin = HexStringToInteger(Text.substr(start, end - start));

    start = Text.find_first_not_of(L" \t", token + 1);

    if (start == wstring::npos) {
      return;
    }
    end = Text.find_first_of(L" \t,", start + 1);

    if (end == wstring::npos) {
      end = Text.size();
    }
    uint final = HexStringToInteger(Text.substr(start, end - start));

    if (final >= begin) {
      uint idx = 0;

      Items.resize(final - begin + 1);

      for (; begin <= final; ++begin) {
        Items[idx++] = begin;
      }
    }
  }
  else {
    if (!Text.empty()) {
      Items.push_back(HexStringToInteger(Text));
    }
  }
}
//-----------------------------------------------------------------------------
template <class T>
void AddOpStrToInts(const string& Text, vector<T>& Items)
{
  Items.clear();

  string::size_type  token = Text.find('+', 0);

  if (token != string::npos) {
    string::size_type start = Text.find_first_not_of(" \t", 0);
    string::size_type end = Text.find_first_of("+ \t,", start + 1);

    if (end == string::npos) {
      return;
    }
    vector<T>  begin;

    ColonStrToInts(Text.substr(start, end - start), begin);

    start = Text.find_first_not_of(" \t", token + 1);

    if (start == string::npos) {
      return;
    }
    end = Text.find_first_of("* \t,", start + 1);

    if (end == string::npos) {
      return;
    }
    uint inc = StringToInteger(Text.substr(start, end - start));

    token = Text.find('*', 0);

    if (token == string::npos) {
      return;
    }

    start = Text.find_first_not_of(" \t", token + 1);

    if (start == string::npos) {
      return;
    }
    end = Text.find_first_of(" \t,", start + 1);

    if (end == string::npos) {
      end = Text.size();
    }
    uint iter  = 0;
    uint size  = begin.size();
    uint count = StringToInteger(Text.substr(start, end - start));

    Items.resize(count * size);

    for (uint idx = 0; idx < size; ++idx) {
      start = begin[idx];

      for (uint num = count; num; --num, start += inc) {
        Items[iter++] = start;
      }
    }
  }
}
//-----------------------------------------------------------------------------
template <class T>
void AddOpStrToInts(const wstring& Text, vector<T>& Items)
{
  Items.clear();

  wstring::size_type  token = Text.find(L'+', 0);

  if (token != wstring::npos) {
    wstring::size_type start = Text.find_first_not_of(L" \t", 0);
    wstring::size_type end = Text.find_first_of(L"+ \t,", start + 1);

    if (end == wstring::npos) {
      return;
    }
    vector<T>  begin;

    ColonStrToInts(Text.substr(start, end - start), begin);

    start = Text.find_first_not_of(L" \t", token + 1);

    if (start == wstring::npos) {
      return;
    }
    end = Text.find_first_of(L"* \t,", start + 1);

    if (end == wstring::npos) {
      return;
    }
    uint inc = StringToInteger(Text.substr(start, end - start));

    token = Text.find(L'*', 0);

    if (token == wstring::npos) {
      return;
    }

    start = Text.find_first_not_of(L" \t", token + 1);

    if (start == wstring::npos) {
      return;
    }
    end = Text.find_first_of(L" \t,", start + 1);

    if (end == wstring::npos) {
      end = Text.size();
    }
    uint iter  = 0;
    uint size  = begin.size();
    uint count = StringToInteger(Text.substr(start, end - start));

    Items.resize(count * size);

    for (uint idx = 0; idx < size; ++idx) {
      start = begin[idx];

      for (uint num = count; num; --num, start += inc) {
        Items[iter++] = start;
      }
    }
  }
}
//-----------------------------------------------------------------------------
template <class T>
void AddOpHexStrToInts(const string& Text, vector<T>& Items)
{
  Items.clear();

  string::size_type  token = Text.find('+', 0);

  if (token != string::npos) {
    string::size_type start = Text.find_first_not_of(" \t", 0);
    string::size_type end = Text.find_first_of("+ \t,", start + 1);

    if (end == string::npos) {
      return;
    }
    vector<T>  begin;

    ColonHexStrToInts(Text.substr(start, end - start), begin);

    start = Text.find_first_not_of(" \t", token + 1);

    if (start == string::npos) {
      return;
    }
    end = Text.find_first_of("* \t,", start + 1);

    if (end == string::npos) {
      return;
    }
    uint inc = HexStringToInteger(Text.substr(start, end - start));

    token = Text.find('*', 0);

    if (token == string::npos) {
      return;
    }

    start = Text.find_first_not_of(" \t", token + 1);

    if (start == string::npos) {
      return;
    }
    end = Text.find_first_of(" \t,", start + 1);

    if (end == string::npos) {
      end = Text.size();
    }
    uint iter  = 0;
    uint size  = begin.size();
    uint count = HexStringToInteger(Text.substr(start, end - start));

    Items.resize(count * size);

    for (uint idx = 0; idx < size; ++idx) {
      start = begin[idx];

      for (uint num = count; num; --num, start += inc) {
        Items[iter++] = start;
      }
    }
  }
}
//-----------------------------------------------------------------------------
template <class T>
void AddOpHexStrToInts(const wstring& Text, vector<T>& Items)
{
  Items.clear();

  wstring::size_type  token = Text.find('+', 0);

  if (token != wstring::npos) {
    wstring::size_type start = Text.find_first_not_of(L" \t", 0);
    wstring::size_type end = Text.find_first_of(L"+ \t,", start + 1);

    if (end == wstring::npos) {
      return;
    }
    vector<T>  begin;

    ColonHexStrToInts(Text.substr(start, end - start), begin);

    start = Text.find_first_not_of(L" \t", token + 1);

    if (start == wstring::npos) {
      return;
    }
    end = Text.find_first_of(L"* \t,", start + 1);

    if (end == wstring::npos) {
      return;
    }
    uint inc = HexStringToInteger(Text.substr(start, end - start));

    token = Text.find('*', 0);

    if (token == wstring::npos) {
      return;
    }

    start = Text.find_first_not_of(L" \t", token + 1);

    if (start == string::npos) {
      return;
    }
    end = Text.find_first_of(L" \t,", start + 1);

    if (end == wstring::npos) {
      end = Text.size();
    }
    uint iter  = 0;
    uint size  = begin.size();
    uint count = HexStringToInteger(Text.substr(start, end - start));

    Items.resize(count * size);

    for (uint idx = 0; idx < size; ++idx) {
      start = begin[idx];

      for (uint num = count; num; --num, start += inc) {
        Items[iter++] = start;
      }
    }
  }
}
//-----------------------------------------------------------------------------
bool ErasePartitionChar(string& Text);
bool ErasePartitionChar(wstring& Text);
//-----------------------------------------------------------------------------
template <class T>
void SimpleRegExStrToInts(const string& Text, vector<T>& Ints)
{
  Ints.clear();

  string  str(Text);

  if (!ErasePartitionChar(str)) {
    throw CError(L"RegExStrToInts() fail.");
  }
  vector<T>       temp;
  vector<string>  items;

  SplitString(str, ", \t", items);

  uint itemCount = items.size();

  for (uint idx = 0; idx < itemCount; ++idx) {
    string&  subStr = items[idx];

    if (subStr.find('+') != string::npos) {
      AddOpStrToInts(subStr, temp);
    }
    else {
      if (subStr.find(':') != string::npos) {
        ColonStrToInts(subStr, temp);
      }
      else {
        temp.resize(1);
        temp[0] = StringToInteger(subStr);
      }
    }
    uint size = Ints.size();

    Ints.resize(size + temp.size());

    copy(temp.begin(), temp.end(), Ints.begin() + size);
  }
  sort(Ints.begin(), Ints.end());
  Ints.erase(unique(Ints.begin(), Ints.end()), Ints.end());
}
//-----------------------------------------------------------------------------
template <class T>
void RemoveVector(vector<T>& Source, const vector<T>& Discard)
{
	typename vector<T>::const_iterator it = Discard.begin();
  typename vector<T>::const_iterator end = Discard.end();

  while (it != end) {
		typename vector<T>::iterator endPos = Source.end();
    typename vector<T>::iterator pos = find(Source.begin(), endPos, *it);

    if (pos != endPos) {
      Source.erase(pos);
    }
    ++it;
  }
}
//-----------------------------------------------------------------------------
template <class T>
void RegExStrToInts(const string& Text, vector<T>& Ints)
{
  Ints.clear();

  string  str(Text);

  if (!ErasePartitionChar(str)) {
    throw CError(L"RegExStrToInts() fail.");
  }

  vector<T>       discardInts;
  vector<string>  discardItems;

  SplitString(str, "-", discardItems);

  uint end = discardItems.size();

  if (end > 2) {
    end = 2;
  }
  for (uint from = 0; from < end; ++from) {
    vector<T>       temp;
    vector<string>  items;

    SplitString(discardItems[from], ", \t", items);

    uint itemCount = items.size();

    for (uint idx = 0; idx < itemCount; ++idx) {
      string&  subStr = items[idx];

      if (subStr.find('+') != string::npos) {
        AddOpStrToInts(subStr, temp);
      }
      else {
        if (subStr.find(':') != string::npos) {
          ColonStrToInts(subStr, temp);
        }
        else {
          temp.resize(1);
          temp[0] = StringToInteger(subStr);
        }
      }
      if (from) {
        uint size = discardInts.size();

        discardInts.resize(size + temp.size());

        copy(temp.begin(), temp.end(), discardInts.begin() + size);
      }
      else {
        uint size = Ints.size();

        Ints.resize(size + temp.size());

        copy(temp.begin(), temp.end(), Ints.begin() + size);
      }
    }
  }
  sort(Ints.begin(), Ints.end());
  Ints.erase(unique(Ints.begin(), Ints.end()), Ints.end());
  RemoveVector(Ints, discardInts);
}
//-----------------------------------------------------------------------------
template <class T>
void RegExStrToInts(const wstring& Text, vector<T>& Ints)
{
  Ints.clear();

  wstring  str(Text);

  if (!ErasePartitionChar(str)) {
    throw CError(L"RegExStrToInts() fail.");
  }

  vector<T>        discardInts;
  vector<wstring>  discardItems;

  SplitString(str, L"-", discardItems);

  uint end = discardItems.size();

  if (end > 2) {
    end = 2;
  }
  for (uint from = 0; from < end; ++from) {
    vector<T>        temp;
    vector<wstring>  items;

    SplitString(discardItems[from], L", \t", items);

    uint itemCount = items.size();

    for (uint idx = 0; idx < itemCount; ++idx) {
      wstring&  subStr = items[idx];

      if (subStr.find(L'+') != wstring::npos) {
        AddOpStrToInts(subStr, temp);
      }
      else {
        if (subStr.find(L':') != wstring::npos) {
          ColonStrToInts(subStr, temp);
        }
        else {
          temp.resize(1);
          temp[0] = StringToInteger(subStr);
        }
      }
      if (from) {
        uint size = discardInts.size();

        discardInts.resize(size + temp.size());

        copy(temp.begin(), temp.end(), discardInts.begin() + size);
      }
      else {
        uint size = Ints.size();

        Ints.resize(size + temp.size());

        copy(temp.begin(), temp.end(), Ints.begin() + size);
      }
    }
  }
  sort(Ints.begin(), Ints.end());
  Ints.erase(unique(Ints.begin(), Ints.end()), Ints.end());
  RemoveVector(Ints, discardInts);
}
//-----------------------------------------------------------------------------
template <class T>
void RegExHexStrToInts(const string& Text, vector<T>& Ints, bool SortInts = true)
{
  Ints.clear();

  string  str(Text);

  if (!ErasePartitionChar(str)) {
    throw CError(L"RegExHexStrToInts() fail.");
  }

  vector<T>       discardInts;
  vector<string>  discardItems;

  SplitString(str, "-", discardItems);

  uint end = discardItems.size();

  if (end > 2) {
    end = 2;
  }
  for (uint from = 0; from < end; ++from) {
    vector<T>       temp;
    vector<string>  items;

    SplitString(discardItems[from], ", \t", items);

    uint itemCount = items.size();

    for (uint idx = 0; idx < itemCount; ++idx) {
      string&  subStr = items[idx];

      if (subStr.find('+') != string::npos) {
        AddOpHexStrToInts(subStr, temp);
      }
      else {
        if (subStr.find(':') != string::npos) {
          ColonHexStrToInts(subStr, temp);
        }
        else {
          temp.resize(1);
          temp[0] = HexStringToInteger(subStr);
        }
      }
      if (from) {
        uint size = discardInts.size();

        discardInts.resize(size + temp.size());

        copy(temp.begin(), temp.end(), discardInts.begin() + size);
      }
      else {
        uint size = Ints.size();

        Ints.resize(size + temp.size());

        copy(temp.begin(), temp.end(), Ints.begin() + size);
      }
    }
  }
  if (SortInts) {
    sort(Ints.begin(), Ints.end());
    Ints.erase(unique(Ints.begin(), Ints.end()), Ints.end());
  }
  RemoveVector(Ints, discardInts);
}
//-----------------------------------------------------------------------------
template <class T>
void RegExHexStrToInts(const wstring& Text, vector<T>& Ints, bool SortInts = true)
{
  Ints.clear();

  wstring  str(Text);

  if (!ErasePartitionChar(str)) {
    throw CError(L"RegExHexStrToInts() fail.");
  }

  vector<T>        discardInts;
  vector<wstring>  discardItems;

  SplitString(str, L"-", discardItems);

  uint end = discardItems.size();

  if (end > 2) {
    end = 2;
  }
  for (uint from = 0; from < end; ++from) {
    vector<T>        temp;
    vector<wstring>  items;

    SplitString(discardItems[from], L", \t", items);

    uint itemCount = items.size();

    for (uint idx = 0; idx < itemCount; ++idx) {
      wstring&  subStr = items[idx];

      if (subStr.find(L'+') != wstring::npos) {
        AddOpHexStrToInts(subStr, temp);
      }
      else {
        if (subStr.find(L':') != wstring::npos) {
          ColonHexStrToInts(subStr, temp);
        }
        else {
          temp.resize(1);
          temp[0] = HexStringToInteger(subStr);
        }
      }
      if (from) {
        uint size = discardInts.size();

        discardInts.resize(size + temp.size());

        copy(temp.begin(), temp.end(), discardInts.begin() + size);
      }
      else {
        uint size = Ints.size();

        Ints.resize(size + temp.size());

        copy(temp.begin(), temp.end(), Ints.begin() + size);
      }
    }
  }
  if (SortInts) {
    sort(Ints.begin(), Ints.end());
    Ints.erase(unique(Ints.begin(), Ints.end()), Ints.end());
  }
  RemoveVector(Ints, discardInts);
}
//-----------------------------------------------------------------------------
class CEndian
{
public :
  word Swap(word Value)
  {
    return ((Value >> 8) + (Value << 8));
  }
  dword Swap(dword Value)
  {
    return ((Value << 24) +
            (Value >> 24) +
            ((Value << 8) & 0x00FF0000) +
            ((Value >> 8) & 0x0000FF00));
  }
  void Swap(word* Buffer, uint Length);
  void Swap(dword* Buffer, uint Length);
};
//-----------------------------------------------------------------------------
wstring ToWideString(const string& Source);
string  ToNarrowString(const wstring& Source);
//-----------------------------------------------------------------------------
enum TSModes{TSMode_Short, TSMode_Normal, TSMode_Full, TSMode_FileName};
//-----------------------------------------------------------------------------
#define CTimeStamp class_TimeStamp::GetInstance()
class class_TimeStamp
{
public:
	// Singleton
	static class_TimeStamp& GetInstance()
	{
		static class_TimeStamp Inst;
		return Inst;
	}

public:
	class_TimeStamp() {}

	wstring str(int mode = TSMode_Short) {
		struct tm * ptm;
		time(&_time);
		ptm = gmtime(&_time);
		int year = ptm->tm_year + 1900;
		int month = ptm->tm_mon + 1;
		int date = ptm->tm_mday;
		int hour = ptm->tm_hour + 8;
		int min = ptm->tm_min;
		int sec = ptm->tm_sec;

		wostringstream  text;

		if (mode == TSMode_Short) {
			year = year - (year / 100) * 100;
			text << setfill(L'0') << setw(2) << year
					 << setfill(L'0') <<setw(2) << month
					 << setfill(L'0') <<setw(2) << date
					 << setfill(L'0') <<setw(2) << hour
					 << setfill(L'0') <<setw(2) << min
					 << setfill(L'0') <<setw(2) << sec;
		}
		else if (mode == TSMode_FileName) {
			text << setfill(L'0') << setw(4) << year
					 << setfill(L'0') <<setw(2) << month
					 << setfill(L'0') <<setw(2) << date
					 << L"_"
					 << setfill(L'0') <<setw(2) << hour
					 << setfill(L'0') <<setw(2) << min
					 << setfill(L'0') <<setw(2) << sec;
		}
		else if (mode == TSMode_Normal){
			text << setfill(L'0') << setw(2) << year
					 << L"-" << setfill(L'0') <<setw(2) << month
					 << L"-" << setfill(L'0') <<setw(2) << date
					 << L" " << setfill(L'0') <<setw(2) << hour
					 << L":" << setfill(L'0') <<setw(2) << min
					 << L":" << setfill(L'0') <<setw(2) << sec;
		}
		return text.str();
	}

private:
	time_t		_time;
};
//-----------------------------------------------------------------------------

#endif
