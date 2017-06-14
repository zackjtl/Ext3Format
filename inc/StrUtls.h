#ifndef StrUtlsH
#define StrUtlsH

#pragma once

#include <locale>
#include <string>
#include <vector>

using namespace std;
#define _GLIBCXX_USE_CXX11_ABI 0
namespace StrUtls {
//---------------------------------------------------------------------------
template <typename T>
void ToLower(basic_string<T>& Source, const locale& Loc=std::locale())
{
  use_facet< ctype<T> >(Loc).tolower(&Source[0], &Source[0] + Source.size());
}
//---------------------------------------------------------------------------
template <typename T>
void ToUpper(basic_string<T>& Source, const locale& Loc=std::locale())
{
  use_facet< ctype<T> >(Loc).toupper(&Source[0], &Source[0] + Source.size());
}
//---------------------------------------------------------------------------
template <typename T>
void TrimLeft(basic_string<T>& Source, const basic_string<T>& SearchPattern)
{
  typename basic_string<T>::size_type pos = Source.find_first_not_of(SearchPattern);

  if (pos && (pos != basic_string<T>::npos)) {
    Source.erase(0, pos);
  }
}
//---------------------------------------------------------------------------
template <typename T>
void TrimLeft(basic_string<T>& Source, const T* SearchPattern)
{
  typename basic_string<T>::size_type pos = Source.find_first_not_of(SearchPattern);

  if (pos && (pos != basic_string<T>::npos)) {
    Source.erase(0, pos);
  }
}
//---------------------------------------------------------------------------
template <typename T>
void TrimRight(basic_string<T>& Source, const basic_string<T>& SearchPattern)
{
  typename basic_string<T>::size_type pos = Source.find_last_not_of(SearchPattern);

  if (pos != basic_string<T>::npos) {
    Source.erase(pos + 1);
  }
  else {
    Source.clear();
  }
}
//---------------------------------------------------------------------------
template <typename T>
void TrimRight(basic_string<T>& Source, const T* SearchPattern)
{
  typename basic_string<T>::size_type pos = Source.find_last_not_of(SearchPattern);

  if (pos != basic_string<T>::npos) {
    Source.erase(pos + 1);
  }
  else {
    Source.clear();
  }
}
//---------------------------------------------------------------------------
template <typename T>
void Trim(basic_string<T>& Source, const basic_string<T>& SearchPattern)
{
  TrimLeft(Source, SearchPattern);
  TrimRight(Source, SearchPattern);
}
//---------------------------------------------------------------------------
template <typename T>
void Trim(basic_string<T>& Source, const T* SearchPattern)
{
  TrimLeft(Source, SearchPattern);
  TrimRight(Source, SearchPattern);
}
//---------------------------------------------------------------------------
template <typename T>
void Split(const basic_string<T>&      Source,
           const basic_string<T>&      Division,
           vector< basic_string<T> >&  Strings)
{
  Strings.clear();

  typename basic_string<T>::size_type start = Source.find_first_not_of(Division, 0);

  while (start != basic_string<T>::npos) {
    typename basic_string<T>::size_type end = Source.find_first_of(Division, start + 1);

    if (end == basic_string<T>::npos) {
      Strings.push_back(Source.substr(start, Source.size() - start));
      break;
    }
    else {
      Strings.push_back(Source.substr(start, end - start));
      start = Source.find_first_not_of(Division, end + 1);
    }
  }
}
//-----------------------------------------------------------------------------
template <typename T>
void Split(const basic_string<T>&      Source,
           const T*                    Division,
           vector< basic_string<T> >&  Strings)
{
  Strings.clear();

  typename basic_string<T>::size_type start = Source.find_first_not_of(Division, 0);

  while (start != basic_string<T>::npos) {
    typename basic_string<T>::size_type end = Source.find_first_of(Division, start + 1);

    if (end == basic_string<T>::npos) {
      Strings.push_back(Source.substr(start, Source.size() - start));
      break;
    }
    else {
      Strings.push_back(Source.substr(start, end - start));
      start = Source.find_first_not_of(Division, end + 1);
    }
  }
}
//-----------------------------------------------------------------------------
#ifndef __GNUC__
wstring TrimSpaces(const wstring& Text)
{
  wstring::size_type  pos = Text.find_first_not_of(L" ");

  if (pos == wstring::npos) {
    return L"";
  }
  else {
    return  Text.substr(pos, Text.find_last_not_of(L" ") + 1);
  }
}
#endif
//-----------------------------------------------------------------------------
};
#endif
