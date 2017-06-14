//---------------------------------------------------------------------------
#ifndef BitArrayH
#define BitArrayH
#include "BaseTypes.h"
#include "Bulk.h"
#include <memory>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
class CBitArray
{
public:
  class reference {
    friend class CBitArray;
  public:
    reference& operator=(bool Value)
    {
      _BitArray.Set(_Position, Value);
      return *this;
    }
    reference& operator=(const reference& BitRef)    {
      _BitArray.Set(_Position, bool(BitRef));
      return *this;
    }
    reference& Flip()
    {
      _BitArray.Flip(_Position);
      return *this;
    }
    bool operator~() const
    {
      return !_BitArray.Test(_Position);
    }
    operator bool() const
    {
      return (_BitArray.Test(_Position));
    }
  private:
    reference(CBitArray& bitArray, uint Position)
      : _BitArray(bitArray),
        _Position(Position)
    {
    }
    uint        _Position;
    CBitArray&  _BitArray;
  };

  CBitArray();
  CBitArray(uint BitCount);
  CBitArray(uint BitCount, bool Value);
  void Init(byte* InBuf, uint Size, uint BitCount);
  CBitArray(const CBitArray& Source);
  CBitArray& Flip();
  CBitArray& Flip(uint Position);
  CBitArray& operator=(const CBitArray& Source);
  bool operator[](uint Position) const;
  reference operator[](uint Position) { return reference(*this, Position); }
  bool operator==(const CBitArray& Right) const;
  bool operator!=(const CBitArray& Right) const;
  CBitArray& operator&=(const CBitArray& Right);
  CBitArray& operator|=(const CBitArray& Right);
  CBitArray& operator^=(const CBitArray& Right);
  CBitArray& operator<<=(uint Position);
  CBitArray  operator<<(uint Position);
  CBitArray& operator>>=(uint Position);
  CBitArray  operator>>(uint Position);
  CBitArray operator~() const;
  void Resize(uint BitCount);
  bool Any() const;
  bool None() const;
  uint Count() const;
  uint Count(uint Position) const;
  uint Count(uint Start, uint BitCount) const;
  bool Test(uint Position) const;
  CBitArray& Set();
  CBitArray& Set(uint Position, bool Value = true);
  CBitArray& Reset();
  CBitArray& Reset(uint Position);
  Bulk<byte>& GetArray();
  const Bulk<byte>& GetArray() const;
  uint GetBitCount() const;
  wstring ToString(uint Indent);
  wstring ToByteString(uint Indent);

  // A template method should be implemented in the .h file
  template <typename T> void FromVector(const vector<T>& Items)
  {
		typename vector<T>::const_iterator it  = Items.begin();
    typename vector<T>::const_iterator end = Items.end();

    Reset();

    while (it != end) {
      Set(*it);
      ++it;
    }
  }
  template <typename T> void ToVector(vector<T>& Items)
  {
    uint iter     = 0;
    uint count    = Count();
    uint bitCount = GetBitCount();

    Items.resize(count);

    for (uint idx = 0; idx < bitCount; ++idx) {
      if (Test(idx)) {
        Items[iter++] = idx;
      }
    }
  }
  void Trim();
  bool IsFullFill();
private:
  uint         _BitCount;
  Bulk<byte>   _Array;
};
//---------------------------------------------------------------------------
CBitArray operator&(const CBitArray& Left, const CBitArray& Right);
CBitArray operator&=(const CBitArray& Left, const CBitArray& Right);
CBitArray operator|=(const CBitArray& Left, const CBitArray& Right);
CBitArray operator^=(const CBitArray& Left, const CBitArray& Right);
CBitArray operator<<=(const CBitArray& Left, const CBitArray& Right);
CBitArray operator>>=(const CBitArray& Left, const CBitArray& Right);
bool operator<(const CBitArray& Left, const CBitArray& Right);
bool operator>(const CBitArray& Left, const CBitArray& Right);
//---------------------------------------------------------------------------
#endif
