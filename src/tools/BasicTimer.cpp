#include "Pch.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "BasicTimer.h"
#include <sstream>
#include <iomanip>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
CBasicTimer::CBasicTimer()
  : _Timeout(0),
    _StartTicks(clock())
{
}
//---------------------------------------------------------------------------
CBasicTimer::CBasicTimer(uint Timeout)
  : _Timeout(Timeout),
    _StartTicks(clock())
{
}
//---------------------------------------------------------------------------
CBasicTimer::~CBasicTimer()
{
}
//---------------------------------------------------------------------------
void CBasicTimer::SetTimeout(uint Value)
{
  _Timeout = Value;
}
//---------------------------------------------------------------------------
uint CBasicTimer::GetTimeout()
{
  return _Timeout;
}
//---------------------------------------------------------------------------
void CBasicTimer::Reset()
{
  _StartTicks = clock();
}
//---------------------------------------------------------------------------
uint CBasicTimer::GetStartTick()
{
  return _StartTicks;
}
//---------------------------------------------------------------------------
uint CBasicTimer::Ticks()
{
  return (uint) clock();
}
//---------------------------------------------------------------------------
uint CBasicTimer::Elapsed()
{
  return (uint) clock() - _StartTicks;
}
//---------------------------------------------------------------------------
bool CBasicTimer::IsOverTime()
{
  return Elapsed() > _Timeout ? true : false;
}
//---------------------------------------------------------------------------
wstring CBasicTimer::ToString()
{
  uint            msec = Elapsed();
  uint            sec  = msec / 1000;
  uint            min  = sec / 60UL % 60UL;
  uint            hour = sec / 60UL / 60UL;
  wostringstream  text;

  sec = sec % 60UL;

  text << right << setfill(L'0');

  if (hour < 10) {
    text << setw(2);
  }
  text << hour << L':'
      << setw(2) << setfill(L'0') << min << L':'
      << setw(2) << setfill(L'0') << sec << L'.'
      << setw(3) << setfill(L'0') << (msec % 1000);

  return text.str();
}
//---------------------------------------------------------------------------
