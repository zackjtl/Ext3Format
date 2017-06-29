//---------------------------------------------------------------------------
#ifndef E2fsBlockIoH
#define E2fsBlockIoH
//---------------------------------------------------------------------------
#include "BlockManager.h"
#include "UsbDrive.h"
#include <vector>
//---------------------------------------------------------------------------
class CE2fsBlockIo
{
public:
  CE2fsBlockIo(CBlockManager& BlockMan, CUsbDrive* Drive, bool CheckMode);
  ~CE2fsBlockIo();

  void ResetQueue();
  void SetQueueBmp(std::vector<byte>& Bmp);  
  void AddBlockToQueue(uint32 Block);  
  void SetArrayToQueue(std::vector<uint32>& Array);

  void WriteBlocksInQueue();
  void ReadBlocksInQueue();

private:
  void WriteArea(uint32 StartBlock, uint32 Count);
  void ReadArea(uint32 StartBlock, uint32 Count);

private:
  CBlockManager&  _BlockMan;
  CUsbDrive*      _Drive;
  uint32          _BlockSize;
  bool            _CheckMode;

  std::vector<byte>    _QueueBmp;
};
//---------------------------------------------------------------------------
#endif
