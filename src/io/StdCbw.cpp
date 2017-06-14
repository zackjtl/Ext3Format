//---------------------------------------------------------------------------
#include "Pch.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "StdCbw.h"
//---------------------------------------------------------------------------
CCbwParam::CCbwParam()
{
}
//---------------------------------------------------------------------------
CCbwParam::~CCbwParam()
{
}
//---------------------------------------------------------------------------
void CCbwParam::Reset()
{
  ZeroMemory( static_cast<PVOID>(_Cbwcb.Cbd) , sizeof(_Cbwcb));
  _CbwcbSet.clear();
  ZeroMemory(CmdWrapper,sizeof(CmdWrapper)) ;
  _Length.ul = 0;
  _CbdLen.ui = 0;
}
//---------------------------------------------------------------------------
void CCbwParam::SetTransLenInByte( ULONG Length )
{
  _Length.ul = Length ;
}   // little-endia
//---------------------------------------------------------------------------
void CCbwParam::SetFlag(bool Flag)
{
  _Flag = Flag ;
}
//---------------------------------------------------------------------------
void CCbwParam::SetLun(BYTE Lun)
{
  _Lun = Lun;
}
//---------------------------------------------------------------------------
void CCbwParam::SetCbdLen(UINT Length)
{
  _CbdLen.ui = Length ;
}
//---------------------------------------------------------------------------
void CCbwParam::SetCbd( UINT Index , BYTE Value)
{
  _Cbwcb.Cbd[Index] = Value;
}
//---------------------------------------------------------------------------
void CCbwParam::SetOpCode( UCHAR Op, UCHAR SubOp)
{
  _Cbwcb.Para.op = Op , _Cbwcb.Para.SubOp = SubOp;
}
//---------------------------------------------------------------------------
void CCbwParam::SetLba(ULONG Lba)
{
  _Cbwcb.Para.LBA.ul = Lba;
}
//---------------------------------------------------------------------------
void CCbwParam::SetCount(UINT Len)
{
  _Cbwcb.Para.Length.ui = Len;
}
//---------------------------------------------------------------------------
void CCbwParam::SetArg(ULONG Arg)
{
  _Cbwcb.Para.Arg.ul = Arg;
}
//---------------------------------------------------------------------------
dword CCbwParam::GetTransLenInByte()
{
  return _Length.ul;
}
//---------------------------------------------------------------------------
bool CCbwParam::GetFlag()
{
  return _Flag;
}
//---------------------------------------------------------------------------
BYTE CCbwParam::GetLun()
{
  return _Lun;
}
//---------------------------------------------------------------------------
UINT CCbwParam::GetCbdLen()
{
  return _CbdLen.ui;
}
/*
//---------------------------------------------------------------------------
vector<byte>& CCbwParam::GetCbd()
 {
   //vector<byte> CdbTemp(&_Cbwcb.Cbd[0],&_Cbwcb.Cbd[11]);
   _CbwcbSet.assign(&_Cbwcb.Cbd[0],&_Cbwcb.Cbd[11]);

   return _CbwcbSet;
}*/
//---------------------------------------------------------------------------
byte* CCbwParam::GetCbd()
{
  return &_Cbwcb.Cbd[0];
}
//---------------------------------------------------------------------------
void CCbwParam::GetCbd(byte* OutBuf, uint Count)
{
  memcpy(OutBuf, &_Cbwcb.Cbd[0], Count);
}
//---------------------------------------------------------------------------
PCHAR CCbwParam::GetCmdWrapper()
 {

    ZeroMemory(CmdWrapper,sizeof(CmdWrapper));

    CmdWrapper[0]=0x55;  // Signature
    CmdWrapper[1]=0x53;
    CmdWrapper[2]=0x42;
    CmdWrapper[3]=0x43;

    CmdWrapper[4]=0x08; // Tag
    CmdWrapper[5]=0x90;
    CmdWrapper[6]=0xF1;
    CmdWrapper[7]=0x81;

    CmdWrapper[8]= _Length.uc[0];  // data reansfer length (Little-endia)
    CmdWrapper[9]=_Length.uc[1];
    CmdWrapper[0x0a]= 0;//_Length.uc[2];
    CmdWrapper[0x0b]= 0;//_Length.uc[3];

    if(_Flag)                           // CBW Flag
       CmdWrapper[0x0c]=0x80;
    else
       CmdWrapper[0x0c]=0x00;

     CmdWrapper[0x0d]=_Lun;         // loguical unit numver
     CmdWrapper[0x0e]=_CbdLen.uc[0];// CBWCB LENGTH


     for(UINT i=0 ; i<12 ; ++i )       // CBD
     {
       CmdWrapper[0x0f+i]=_Cbwcb.Cbd[i] ;
     }

    return CmdWrapper;


 }

//---------------------------------------------------------------------------
