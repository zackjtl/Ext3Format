//---------------------------------------------------------------------------
#ifndef BasicTimerH
#define BasicTimerH
//---------------------------------------------------------------------------
#include "BaseTypes.h"
#include <ctime>
#include <string>

using namespace std;
//---------------------------------------------------------------------------
class CBasicTimer
{
public:
  CBasicTimer();
  CBasicTimer(uint Timeout);
  ~CBasicTimer();

  void    SetTimeout(uint Value);
  uint    GetTimeout();
  void    Reset();
  uint    Elapsed();
  bool    IsOverTime();
  wstring ToString();

	uint GetStartTick();
	uint Ticks();
private:
  uint  _Timeout;
  uint  _StartTicks;
};
//---------------------------------------------------------------------------
#endif
