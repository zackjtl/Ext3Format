#pragma hdrstop
//---------------------------------------------------------------------------
#include "BitArray.h"
#include <iomanip>
#include <sstream>
//---------------------------------------------------------------------------
CBitArray::CBitArray()
  : _BitCount(0)
{
}
//---------------------------------------------------------------------------
CBitArray::CBitArray(uint BitCount)
{
  Resize(BitCount);
}
//---------------------------------------------------------------------------
CBitArray::CBitArray(uint BitCount, bool Value)
{
  Resize(BitCount);

  if (Value) {
    Set();
  }
  else {
    Reset();
  }
}
//---------------------------------------------------------------------------
void CBitArray::Init(byte* InBuf, uint Size, uint BitCount)
{
  Resize(BitCount);

  if (Size > _Array.Size()) {
    Size = _Array.Size();
  }
  memcpy(_Array.Data(), InBuf, Size);
}
//---------------------------------------------------------------------------
CBitArray::CBitArray(const CBitArray& Source)
  : _Array(Source._Array),
    _BitCount(Source._BitCount)
{
}
//---------------------------------------------------------------------------
CBitArray& CBitArray::Flip(uint Position)
{
  return Set(Position, !Test(Position));
}
//---------------------------------------------------------------------------
CBitArray& CBitArray::operator=(const CBitArray& Source)
{
  if (&Source != this) {
    _Array    = Source._Array;
    _BitCount = Source._BitCount;
  }
  return *this;
}
//---------------------------------------------------------------------------
bool CBitArray::operator[](uint Position) const
{
  return Test(Position);
}
//---------------------------------------------------------------------------
CBitArray CBitArray::operator<<(uint Position)
{
  return CBitArray(*this) <<= Position;
}
//---------------------------------------------------------------------------
CBitArray CBitArray::operator>>(uint Position)
{
  return CBitArray(*this) >>= Position;
}
//---------------------------------------------------------------------------
CBitArray CBitArray::operator~() const
{
  return CBitArray(*this).Flip();
}
//---------------------------------------------------------------------------
void CBitArray::Resize(uint BitCount)
{
  _BitCount = BitCount;
  _Array.Resize((BitCount + 8 - 1) / 8);
}
//---------------------------------------------------------------------------
bool CBitArray::None() const
{
  return !Any();
}
//---------------------------------------------------------------------------
CBitArray& CBitArray::Set()
{
  memset(_Array.Data(), 0xff, _Array.Size());
  Trim();
  return *this;
}
//---------------------------------------------------------------------------
CBitArray& CBitArray::Reset()
{
  memset(_Array.Data(), 0, _Array.Size());
  return *this;
}
//---------------------------------------------------------------------------
CBitArray& CBitArray::Reset(uint Position)
{
  return Set(Position, false);
}
//---------------------------------------------------------------------------
Bulk<byte>& CBitArray::GetArray()
{
  return _Array;
}
//---------------------------------------------------------------------------
const Bulk<byte>& CBitArray::GetArray() const
{
  return _Array;
}
//---------------------------------------------------------------------------
uint CBitArray::GetBitCount() const
{
  return _BitCount;
}
//---------------------------------------------------------------------------
bool CBitArray::Any() const
{
  dword*  dwordPtr   = (dword*) _Array.Data();

  for (uint count = _BitCount / 32; count; --count) {
    if (*dwordPtr) {
      return true;
    }
    ++dwordPtr;
  }
  uint remain = _BitCount % 32;

  if (remain) {
    byte* bytePtr = (byte*) dwordPtr;

    for (uint count = remain / 8; count; --count) {
      if (*bytePtr) {
        return true;
      }
      ++bytePtr;
    }
    uint value = *bytePtr;

    remain %= 8;

    for (uint idx = 0; idx < remain; ++idx) {
      if (value & (1 << idx)) {
        return true;
      }
    }
  }
  return false;
}
//---------------------------------------------------------------------------
uint CBitArray::Count() const
{
  uint    count    = 0;
  dword*  dwordPtr = (dword*) _Array.Data();

  for (uint size = _BitCount / 32; size; --size) {
    dword  value = *dwordPtr++;

    if (value == 0xffffffff) {
      count += 32;
    }
    else {
      for (uint idx = 0; idx < 32; ++idx) {
        if (value & (1 << idx)) {
          ++count;
        }
      }
    }
  }
  uint remain = _BitCount % 32;

  if (remain) {
    byte* bytePtr = (byte*) dwordPtr;
    dword value;

    for (uint size = remain / 8; size; --size) {
      value = *bytePtr++;

      for (uint idx = 0; idx < remain; ++idx) {
        if (value & (1 << idx)) {
          ++count;
        }
      }
    }
    remain %= 8;

    if (remain) {
      value = *bytePtr;

      for (uint idx = 0; idx < remain; ++idx) {
        if (value & (1 << idx)) {
          ++count;
        }
      }
    }
  }
  return count;
}
//---------------------------------------------------------------------------
uint CBitArray::Count(uint Position) const
{
  uint    count    = 0;
  uint    bitCount = Position + 1;
  dword*  dwordPtr = (dword*) _Array.Data();

  for (uint size = bitCount / 32; size; --size) {
    dword  value = *dwordPtr++;

    if (value == 0xffffffff) {
      count += 32;
    }
    else {
      for (uint idx = 0; idx < 32; ++idx) {
        if (value & (1 << idx)) {
          ++count;
        }
      }
    }
  }
  uint remain = bitCount % 32;

  if (remain) {
    byte* bytePtr = (byte*) dwordPtr;
    dword value;

    for (uint size = remain / 8; size; --size) {
      value = *bytePtr++;

      for (uint idx = 0; idx < remain; ++idx) {
        if (value & (1 << idx)) {
          ++count;
        }
      }
    }
    value = *bytePtr;

    remain %= 8;

    for (uint idx = 0; idx < remain; ++idx) {
      if (value & (1 << idx)) {
        ++count;
      }
    }
  }
  return count;
}
//---------------------------------------------------------------------------
uint CBitArray::Count(uint Start, uint BitCount) const
{
  uint   count  = 0;
  byte*  buffer = _Array.Data() + Start / 8;
  uint   offset = Start % 8;

  while (BitCount) {
    byte value = *buffer++;

    for (; (offset < 8) && BitCount; ++offset, --BitCount) {
      if (value & (1 << offset)) {
        ++count;
      }
    }
  }
  return count;
}
//---------------------------------------------------------------------------
bool CBitArray::Test(uint Position) const
{
  if (Position >= _BitCount) {
    return false;
  }
  return _Array[Position / 8] & (1 << (Position % 8)) ? true : false;
}
//---------------------------------------------------------------------------
CBitArray& CBitArray::Set(uint Position, bool Value)
{
  if (Position < _BitCount) {
    byte& item   = _Array[Position / 8];
    uint  bitPos = Position % 8;

    if (Value) {
      item |= 1 << bitPos;
    }
    else {
      item &= ~(1 << bitPos);
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
CBitArray& CBitArray::Flip()
{
  uint size = _Array.Size();

  for (uint idx = 0; idx < size; ++idx) {
    _Array[idx] = ~_Array[idx];
  }
  Trim();

  return *this;
}
//---------------------------------------------------------------------------
bool CBitArray::operator==(const CBitArray& Right) const
{
  return !(*this != Right);
}
//---------------------------------------------------------------------------
bool CBitArray::operator!=(const CBitArray& Right) const
{
  if (_BitCount != Right._BitCount) {
    return true;
  }
  if (_Array != Right._Array) {
    return true;
  }
  return false;
}
//---------------------------------------------------------------------------
CBitArray& CBitArray::operator&=(const CBitArray& Right)
{
  uint size = _Array.Size();
  uint count = Right.GetArray().Size();

  if (size > count) {
    size = count;
  }
  for (uint idx = 0; idx < size; ++idx) {
    _Array[idx] &= Right._Array[idx];
  }
  return *this;
}
//---------------------------------------------------------------------------
CBitArray& CBitArray::operator|=(const CBitArray& Right)
{
  uint size  = _Array.Size();
  uint count = Right.GetArray().Size();

  if (size > count) {
    size = count;
  }
  for (uint idx = 0; idx < size; ++idx) {
    _Array[idx] |= Right._Array[idx];
  }
  Trim();

  return *this;
}
//---------------------------------------------------------------------------
CBitArray& CBitArray::operator^=(const CBitArray& Right)
{
  uint size = _Array.Size();
  uint count = Right.GetArray().Size();

  if (size > count) {
    size = count;
  }
  for (uint idx = 0; idx < size; ++idx) {
    _Array[idx] ^= Right._Array[idx];
  }
  Trim();

  return *this;
}
//---------------------------------------------------------------------------
CBitArray& CBitArray::operator<<=(uint Position)
{
  uint shift = Position / 8;

  if (shift != 0) {
    for (uint size = _Array.Size(); size; --size) {
      uint  idx = size - 1;

      if (size > shift) {
        _Array[idx] = _Array[idx - shift];
      }
      else {
        _Array[idx] = 0;
      }
    }
  }
  Position %= 8;

  if (Position) {
    for (uint size = _Array.Size(); size; --size) {
      uint  idx = size - 1;

      if (idx) {
        _Array[idx] = (_Array[idx] << Position) +
                      (_Array[idx] >> (8 - Position));
      }
      else {
        _Array[idx] <<= Position;
      }
    }
  }
  Trim();

  return *this;
}
//---------------------------------------------------------------------------
CBitArray& CBitArray::operator>>=(uint Position)
{
  uint shift = Position / 8;
  uint size  = _Array.Size();

  if (shift != 0) {
    uint end = size - shift;

    for (uint idx = 0; idx < size; ++idx) {
      if (idx < end) {
        _Array[idx] = _Array[idx + shift];
      }
      else {
        _Array[idx] = 0;
      }
    }
  }
  Position %= 8;

  if (Position) {
    uint end = size - 1;

    for (uint idx = 0; idx < size; ++idx) {
      if (idx < end) {
        _Array[idx] = (_Array[idx] >> Position) +
                      (_Array[idx] << (8 - Position));
      }
      else {
        _Array[idx] >>= Position;
      }
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
bool CBitArray::IsFullFill()
{
  if (Count() == _BitCount) {
    return true;
  }
  return false;
}
//---------------------------------------------------------------------------
void CBitArray::Trim()
{
  byte mask[] = { 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff };

  uint remain = _BitCount % 8;

  if (remain) {
    _Array[_Array.Size() - 1] &= mask[remain - 1];
  }
}
//---------------------------------------------------------------------------
wstring CBitArray::ToString(uint Indent)
{
  wstring         spaces(Indent, L' ');
  wostringstream  text;

  for (uint size = _Array.Size(); size; --size) {
    uint  value = _Array[size - 1];

    text << spaces;

    for (uint pos = 0; pos < 8; ++pos) {
      text << ((value & (0x80 >> pos)) ? L'1' : L'0');

      if (pos == 3) {
        text << L" - ";
      }
    }
    text << L"\r\n";
  }
  return text.str();
}
//---------------------------------------------------------------------------
wstring CBitArray::ToByteString(uint Indent)
{
  uint            idx;
  wstring         spaces(Indent, L' ');
  uint            size = _Array.Size();
  uint            rowCount = (size + 15) / 16;
  byte*           ptr = _Array.Data();
  wostringstream  text;

  text << hex << right;

  for (uint row = 0; row < rowCount; ++row) {
    text << spaces;
    for (uint count = size > 16 ? 16 : size; count; --count) {
      text << setw(2) << setfill(L'0') << (int) *ptr++ << L' ';
    }
    text << L"\r\n";
  }

  return text.str();
}
//---------------------------------------------------------------------------
CBitArray operator&(const CBitArray& Left, const CBitArray& Right)
{
  CBitArray  temp(Left);

  return temp &= Right;
}
//---------------------------------------------------------------------------
CBitArray operator&=(const CBitArray& Left, const CBitArray& Right)
{
  return Left &= Right;
}
//---------------------------------------------------------------------------
CBitArray operator|=(const CBitArray& Left, const CBitArray& Right)
{
  return Left |= Right;
}
//---------------------------------------------------------------------------
CBitArray operator^=(const CBitArray& Left, const CBitArray& Right)
{
  return Left ^= Right;
}
//---------------------------------------------------------------------------
CBitArray operator<<=(const CBitArray& Left, const CBitArray& Right)
{
  return Left <<= Right;
}
//---------------------------------------------------------------------------
CBitArray operator>>=(const CBitArray& Left, const CBitArray& Right)
{
  return Left >>= Right;
}
//---------------------------------------------------------------------------
bool operator<(const CBitArray& Left, const CBitArray& Right)
{
  return Left.Count() < Right.Count();
}
//---------------------------------------------------------------------------
bool operator>(const CBitArray& Left, const CBitArray& Right)
{
  return Left.Count() > Right.Count() ? true : false;
}
//---------------------------------------------------------------------------
