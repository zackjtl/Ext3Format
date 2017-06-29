//---------------------------------------------------------------------------
#include "Pch.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "E2fsBlockIo.h"
#include "fs_assert.h"
#include "GlobalDef.h"

using namespace std;
//---------------------------------------------------------------------------
CE2fsBlockIo::CE2fsBlockIo(CBlockManager& BlockMan, CUsbDrive* Drive, bool CheckMode)
  : _BlockMan(BlockMan),
    _Drive(Drive),
    _BlockSize(BlockMan.GetBlockSize())
{
  _QueueBmp.resize(BlockMan.GetBlockBmp().size(), 0);  
}
//---------------------------------------------------------------------------
CE2fsBlockIo::~CE2fsBlockIo()
{

}

#define occupied_test(x) (occupied_bmp[x / 8] & (0x01 << (x % 8)))
#define queue_test(x) (_QueueBmp[x / 8] & (0x01 << (x % 8)))
#define rd_valid(x) (occupied_test(x) && queue_test(x))

void CE2fsBlockIo::ResetQueue()
{
  memset(&_QueueBmp[0], 0, _QueueBmp.size());
}

void CE2fsBlockIo::SetQueueBmp(std::vector<byte>& Bmp)
{  
  _QueueBmp = Bmp;
  assert(_QueueBmp.size() == _BlockMan.GetBlockBmp().size());
}

void CE2fsBlockIo::AddBlockToQueue(uint32 Block)
{
  if (_CheckMode) {
    vector<byte>& occupied_bmp = _BlockMan.GetBlockBmp();
    fs_assert_true(occupied_test(Block));
  }
  _QueueBmp[Block / 8] |= (0x01 << (Block % 8));
}

void CE2fsBlockIo::SetArrayToQueue(std::vector<uint32>& Array)
{
  uint count = Array.size();
  vector<byte>& occupied_bmp = _BlockMan.GetBlockBmp();

  for (uint32 idx = 0; idx < count; ++idx) {
    if (_CheckMode) {
      fs_assert_true(occupied_test(Array[idx]));  
    }
    _QueueBmp[Array[idx] / 8] |= (0x01 << (Array[idx] % 8));
  }
}


/*
 *	Real write file system into the storage.
 */
void CE2fsBlockIo::WriteBlocksInQueue()
{
  vector<byte>&	occupied_bmp = _BlockMan.GetBlockBmp();

	#define wr_valid(x) (occupied_test(x) && queue_test(x))

	const uint32 frag_window = 4;
  
  uint32 max_blocks = _BlockMan.GetTotalBlocks();
	uint32 window_cnt = div_ceil(max_blocks, frag_window);

	for (uint32 window = 0; window < window_cnt; ++window) {
		uint32 offset = window * frag_window;
		uint32 end = offset + frag_window;

		uint32 cont_start = 0;
		uint32 cont_cnt = 0;
		bool previous = false;

		for (uint32 block = offset; block < end; ++block) {
			if (wr_valid(block)) {
				if (previous) {
					cont_cnt++;
				}
				else {
					cont_start = block;
					cont_cnt = 1;
					previous = true;
				}
			}
			else {
				if (cont_cnt) {
					// WriteBlocksInQueue continuous area..
         	WriteArea(cont_start, cont_cnt);
					cont_cnt = 0;
					previous  = false;
				}

      	previous = false;
			}
		}
		if (cont_cnt) {
			// WriteBlocksInQueue continuous area..
			WriteArea(cont_start, cont_cnt);
		}
	}
}

/*
 *	Write continuous area within a specific fragment length
 */
void CE2fsBlockIo::WriteArea(uint32 StartBlock, uint32 Count)
{
	Bulk<byte>	buffer(Count * _BlockSize);

	_BlockMan.GetBlockData(StartBlock, buffer.Data(), buffer.Size());

	uint32 sectorsPerBlock = _BlockSize / 512;
	uint32 sector_pos = StartBlock * sectorsPerBlock;
	uint32 sector_cnt = Count * sectorsPerBlock;

	_Drive->WriteSector(sector_pos, sector_cnt, buffer.Data());
}


/*
 *  Preset blocks to read. If the blocks are continuous,  the performance will be better
 *   than read single by single blocks.
 */
void CE2fsBlockIo::ReadBlocksInQueue()
{
	const uint32 frag_window = 4;
	vector<byte>&	occupied_bmp = _BlockMan.GetBlockBmp();

  uint32 max_blocks = _BlockMan.GetTotalBlocks();

	uint32 window_cnt = div_ceil(max_blocks, frag_window);
	uint32 remain = max_blocks;

	for (uint32 window = 0; window < window_cnt; ++window) {
		uint32 offset = window * frag_window;
		uint32 end = offset + frag_window;

		uint32 cont_start = 0;
		uint32 cont_cnt = 0;
		bool previous = false;

		for (uint32 block = offset; block < end; ++block) {
			if (rd_valid(block)) {
				if (previous) {
					cont_cnt++;
				}
				else {
					cont_start = block;
					cont_cnt = 1;
					previous = true;
				}
			}
			else {
				if (cont_cnt) {
					// WriteBlocksInQueue continuous area..
         	ReadArea(cont_start, cont_cnt);
					cont_cnt = 0;
					previous  = false;
				}

      	previous = false;
			}
		}
		if (cont_cnt) {
			// WriteBlocksInQueue continuous area..
			ReadArea(cont_start, cont_cnt);
		}
	}  
}

/*
 *	Read continuous blocks within a range
 */
void CE2fsBlockIo::ReadArea(uint32 StartBlock, uint32 Count)
{
	Bulk<byte>	buffer(Count * _BlockSize);

	uint32 sectorsPerBlock = _BlockSize / 512;
	uint32 sector_pos = StartBlock * sectorsPerBlock;
	uint32 sector_cnt = Count * sectorsPerBlock;

	_Drive->ReadSector(sector_pos, sector_cnt, buffer.Data());
  _BlockMan.SetBlockData(StartBlock, buffer.Data(), buffer.Size());
}

//---------------------------------------------------------------------------
