//---------------------------------------------------------------------------
#ifndef StdCbwH
#define StdCbwH

#ifndef __GNUC__
#include<vcl.h>
#endif

#include <vector>
#include <string>
#include "BaseTypes.h"

using namespace std;
//---------------------------------------------------------------------------
#pragma pack(push) /* push current alignment to stack */
#pragma pack(1) /* set alignment to 1 byte boundary */

typedef struct {
   byte   op;
   byte   SubOp;
   UN_UL  LBA;
   byte   GroupNum;
   UN_INT Length;
   UN_UL  Arg;   // SD argument
} TStdCbw;
//---------------------------------------------------------------------------
typedef union {
   TStdCbw Para;
   UCHAR Cbd[15];
}StdCBWCB;
#pragma pack(pop) /* restore original alignment from stack */
//---------------------------------------------------------------------------
class CCbwParam
{
public:
   CCbwParam();
  ~CCbwParam();
   void Reset();
   void SetTransLenInByte( ULONG Length );   // little-endia
   void SetFlag(bool Flag);
   void SetLun(BYTE Lun);
   void SetCbdLen(UINT Length);
   void SetCbd( UINT Index , BYTE Value);
   void SetOpCode( UCHAR Op, UCHAR SubOp);
   void SetLba(ULONG Lba);
   void SetCount(UINT Len);
   void SetArg(ULONG Arg);

   dword GetTransLenInByte();
   bool GetFlag();
   BYTE GetLun();
   UINT GetCbdLen();
   //vector<byte>& GetCbd();
   byte* GetCbd();
   void GetCbd(byte* OutBuf, uint Count);
   PCHAR GetCmdWrapper();
  //   BYTE GetCbd(UINT Index){return _Cbd[Index];};

private:
   CHAR   CmdWrapper[31];
   UN_UL  _Length;       // USB protocol level    little-endia
   bool   _Flag;         //+12
   byte   _Lun;          //+13 [b3..b0] lower nibble only
   UN_INT _CbdLen;       //+14 [b4..b0]
   StdCBWCB  _Cbwcb;        //+15
   vector<BYTE> _CbwcbSet;
};
//---------------------------------------------------------------------------
#endif
