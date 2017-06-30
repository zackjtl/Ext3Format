#include "BlockManager.h"
#include "BaseError.h"
#include "e3fs_def.h"
#include <cstring>
#include <cassert>

using namespace std;

/*
 * 	[attention] The block management only support 32-bit block addressing.
 *  This will be the bottleneck of the entire size of the file system.
 *	Larger media size is allowed in 64-bit environment but this class should
 *  be modified.
 */
CBlockManager::CBlockManager(uint32 TotalBlocks, uint32 BlockSize)
	: _TotalBlocks(TotalBlocks),
		_BlockSize(BlockSize),
		_BasePtr(0)
{
	uint byteCnt = (TotalBlocks + 7) / 8;

	if (byteCnt > _UsedBmp.max_size()) {
		throw CError(L"Block bit map is too large to the container");
	}
	_UsedBmp.resize(byteCnt, 0x00);
	_WrittenBmp = _UsedBmp;

  uint32 endBit = TotalBlocks % 8;
  uint32 endByte = byteCnt - 1;

  if (endBit != 0) {
    for (uint32 i = endBit; i < 8; ++i) {
      _UsedBmp[endByte] |= (0x01 << i);
    }
  }  
}

CBlockManager::~CBlockManager()
{
}
#define used_test(x) (_UsedBmp[x / 8] & (0x01 << (x % 8)))
#define written_test(x) (_WrittenBmp[x / 8] & (0x01 << (x % 8)))

#define set_used(x) {_UsedBmp[x / 8] |= (0x01 << (x % 8)); }
#define set_written(x) {_WrittenBmp[x / 8] |= (0x01 << (x % 8));}
#define unset_used(x) {_UsedBmp[x / 8] &= ~(0x01 << (x % 8)); }
#define unset_written(x) {_WrittenBmp[x / 8] &= ~(0x01 << (x % 8));}

bool CBlockManager::HasBlockOccupied(uint32 Block)
{
  return used_test(Block);
}

bool CBlockManager::HasBlockWritten(uint32 Block)
{
  return written_test(Block);
}

std::vector<byte>& CBlockManager::GetBlockBmp()
{
  return _UsedBmp;
}

std::vector<byte>& CBlockManager::GetWrittenBmp()
{
	return _WrittenBmp;
}

/* 	Automatically allocate the specific amount of free blocks
 *  The returned value is the remaining blocks that not allocated.
 *  The returned vector is the block that allocated,
 *	which my not blocks with sequentially index.
 */
uint32 CBlockManager::AutoAllocBlock(uint32 RequireCount, vector<uint32>& RetBlocks)
{
	if (_BasePtr >= _TotalBlocks) {
		_BasePtr = 1;
	}
 	uint32 remain = RequireCount;
	uint32 iter = _BasePtr;

	while (remain) {
		if (!used_test(iter)) {
			RetBlocks.push_back(iter);
			set_used(iter);
			--remain;
		}
		++iter;

		if (iter == _TotalBlocks) {
			break;
		}
	}

	if (remain == 0) {
		_BasePtr = iter;
	}
	return remain;
}

/* Automatically allocate a block. */
bool CBlockManager::AutoAllocSingleBlock(uint32& Block)
{
	if (_BasePtr >= _TotalBlocks) {
		_BasePtr = 1;
	}
	uint32 iter = _BasePtr;

	for (uint32 iter = _BasePtr; iter < _TotalBlocks; ++iter) {
		if (!used_test(iter)) {
			Block = iter;
			set_used(iter);
			_BasePtr = iter + 1;
			return true;
		}
	}
	return false;
}

/* 	Occupy specific area of blocks
 * 	If the block has allocated will raise an error.
 */
bool CBlockManager::OccupyBlock(uint32 StartBlock, uint32 Count)
{
	if ((StartBlock + Count) >= _TotalBlocks) {
		throw CError(L"The required block count to occupy exceeds the total blocks");
	}
	uint endBlock = StartBlock + Count - 1;
	bool permit = true;

	for (uint i = StartBlock; i <= endBlock; ++i) {
		if (used_test(i)) {
			permit = false;
			break;
		}
		//set_used(i);
	}
	if (permit)	{
		for (uint i = StartBlock; i <= endBlock; ++i) {
			set_used(i);
		}
	}
	return permit;
}

/*	
 *  Sequentially set data of the blocks started by the start offset block.
 */
void CBlockManager::SetBlockData(uint32 StartBlock, byte* Data, uint32 Length)
{
	uint count = Length / _BlockSize;
  
  vector<uint32>  blocks;

	for (uint i = 0; i < count; ++i) {
		blocks.push_back(i + StartBlock);
  }
  SetBlockData(blocks, Data, Length); 
}

/*	Set data of the blocks in the vector. The data will placed continuously
 *  into the block by the vector oder.
 */
void CBlockManager::SetBlockData(vector<uint32>& Blocks, byte* Data, uint32 Length)
{
	uint count = Blocks.size();

	if ((count * _BlockSize) > Length) {
		throw CError(L"The input data length mismatched to the required block count");
	}

	byte* ptr = Data;

	for (uint i = 0; i < count; ++i) {
		if (Blocks[i] >= _TotalBlocks) {
			throw CError(L"The block address to set data exceeds the total blocks");
		}
		if (!used_test(Blocks[i])) {
			throw CError(L"The block should be allocated or occupied before setting data");
		}
		CDataMap::iterator it = _DataMap.find(Blocks[i]);
		Bulk<byte>* dest;

		if (it != _DataMap.end()) {
			dest = it->second;
		}
		else {
			dest = new Bulk<byte>(_BlockSize);
		}
		if (dest != NULL) {
			memcpy(dest->Data(), ptr, _BlockSize);
			_DataMap.insert(make_pair(Blocks[i], dest));
			set_written(Blocks[i]);
			ptr += _BlockSize;
		}
		else {
			throw CError(L"Attempt to access a null Bulk object");
		}
	}
}

/*	Get block data pasted continuously from the block address in the
 *	input vector that may be ordered randomly.
 */
void CBlockManager::GetBlockData(uint32 StartBlock, byte* Data, uint32 Length)
{
	uint count = Length / _BlockSize;

  vector<uint32>  blocks;

	for (uint i = 0; i < count; ++i) {
		blocks.push_back(i + StartBlock);
  }
  GetBlockData(blocks, Data, Length);
}

void CBlockManager::GetBlockData(vector<uint32>& Blocks, byte* Data, uint32 Length)
{
	uint count = Blocks.size();

	if (Length < (count * _BlockSize)) {
		throw CError(L"The input data buffer length less than the required"
								 L"block count");
	}

	byte* ptr = Data;

	for (uint i = 0; i < count; ++i) {
		if (Blocks[i] >= _TotalBlocks) {
			throw CError(L"The block address to get data exceeds the total blocks");
		}
		if ((!used_test(Blocks[i])) || (!written_test(Blocks[i]))) {
			throw CError(L"The block should be allocated and written before "
									 L"getting data");
		}
		CDataMap::iterator it = _DataMap.find(Blocks[i]);
		Bulk<byte>* src;

		if (it != _DataMap.end()) {
			src = it->second;
		}
		else {
			throw CError(L"The block data can't found in the data container.");
		}
		if (src != NULL) {
			memcpy(ptr, src->Data(), _BlockSize);
			ptr += _BlockSize;
		}
		else
			throw CError(L"Attempt to access a null Bulk object");
	}
}

/*
 *	Set data into the indicated block.
 */
void CBlockManager::SetSingleBlockData(uint32 Block, byte* Data, uint32 Length)
{
	if (Length < _BlockSize) {
		throw CError(L"The input data buffer length less than the required "
								 L"block count");
	}

	CDataMap::iterator it = _DataMap.find(Block);
	Bulk<byte>* dest = NULL;

	if (it != _DataMap.end()) {
		dest = it->second;
	}
	else {
		dest = new Bulk<byte>(_BlockSize);
	}
	if (dest != NULL) {
		memcpy(dest->Data(), Data, _BlockSize);
		_DataMap.insert(make_pair(Block, dest));
		set_written(Block);
	}
	else
		throw CError(L"Attempt to access a null Bulk object in "
								 L"setting single block data");
}
/*
 * 	Get data from the indicated block.
 */
void CBlockManager::GetSingleBlockData(uint32 Block, byte* Data, uint32 Length)
{
	if (Length < _BlockSize) {
		throw CError(L"The input data buffer length less than the required "
								 L"block count");
	}

	CDataMap::iterator it = _DataMap.find(Block);
	Bulk<byte>* src;

	if (it != _DataMap.end()) {
		src = it->second;
	}
	else {
		throw CError(L"The block data can't found in the data container.");
	}
	if (src != NULL) {
		memcpy(Data, src->Data(), _BlockSize);
	}
	else
		throw CError(L"Attempt to access a null Bulk object in "
								 L"getting single block data");
}

/*
 * 	New data buffer and return the buffer pointer.
 */
Bulk<byte>* CBlockManager::CreateSingleBlockDataBuffer(uint32 Block)
{
  Bulk<byte>*	temp = new Bulk<byte>(_BlockSize);
	_DataMap.insert(make_pair(Block, temp));
	set_written(Block);

	return GetSingleBlockDataBuffer(Block);
}

/*
 * 	Get data buffer pointer from the indicated block.
 */
Bulk<byte>* CBlockManager::GetSingleBlockDataBuffer(uint32 Block)
{
	CDataMap::iterator it = _DataMap.find(Block);
	Bulk<byte>* src;

	if (it != _DataMap.end()) {
		src = it->second;
	}
	else {
		return NULL;
		//throw CError(L"The block data can't found in the data container.");
	}
	return src;
}

/*
 *  Initial block bmp by the data from a data of an area.
 */
void CBlockManager::SetAreaBlockBmp(Bulk<byte>& AreaBmp, uint32 BeginBlock, uint32 BlockCount)
{
  uint32 byteOffset = BeginBlock / 8;
  uint32 byteCnt = div_ceil(BlockCount, 8);

  assert(AreaBmp.Size() >= byteCnt);

  memcpy((byte*)&_UsedBmp[byteOffset], AreaBmp.Data(), byteCnt);
}

uint32 CBlockManager::GetTotalBlocks()
{
  return _TotalBlocks;
}

uint32 CBlockManager::GetBlockSize()
{
  return _BlockSize;
}

void CBlockManager::ShiftBasePtr(uint32 Value)
{
  _BasePtr = Value;
}

uint32 CBlockManager::GetBasePtr()
{
  return _BasePtr;
}


