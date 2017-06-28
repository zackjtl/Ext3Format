#ifndef BlockManagerH
#define BlockManagerH

#include "BaseTypes.h"
#include "Bulk.h"
#include <vector>
#include <map>

/*
 * 	[attention] The block management only support 32-bit block addressing.
 *  This will be the bottleneck of the entire size of the file system.
 *	Larger media size is allowed in 64-bit environment but this class should
 *  be modified.
 */

typedef std::map<uint32, Bulk<byte>* > 	CDataMap;

class CBlockManager
{
public:
	CBlockManager(uint32 TotalBlocks, uint32 BlockSize);
	~CBlockManager();

	/* Automatically allocate the specific amount of free blocks
	 *  The return value is the remaining blocks that not allocated */
	uint32 AutoAllocBlock(uint32 RequireCount, std::vector<uint32>& RetBlocks);

	/* Automatically allocate a block. */
	bool AutoAllocSingleBlock(uint32& Block);

	/* Occupy specific area of blocks
	 *	 If the block has allocated will raise an error */
	bool OccupyBlock(uint32 StartBlock, uint32 Count);

  /*  Sequentially set data of the blocks started by the start offset block. */
  void SetBlockData(uint32 StartBlock, byte* Data, uint32 Length);

	/*	Set data of the blocks started by the specific address.
	 *  The blocks must be occupied or allocated before setting data. */
	void SetBlockData(std::vector<uint32>& RetBlocks, byte* Data, uint32 Length);

	/*	Get block data pasted continuously from the block address in the
	 *	input vector that may be ordered randomly.    */
	void GetBlockData(uint32 StartBlock, byte* Data, uint32 Length);
	void GetBlockData(std::vector<uint32>& Blocks, byte* Data, uint32 Length);

	void SetSingleBlockData(uint32 Block, byte* Data, uint32 Length);
	void GetSingleBlockData(uint32 Block, byte* Data, uint32 Length);

  bool HasBlockOccupied(uint32 Block);
  bool HasBlockWritten(uint32 Block);

	Bulk<byte>* CreateSingleBlockDataBuffer(uint32 Block);
	Bulk<byte>* GetSingleBlockDataBuffer(uint32 Block);

  void SetAreaBlockBmp(Bulk<byte>& AreaBmp, uint32 StartBlock, uint32 BlockCount);

	std::vector<byte>& GetBlockBmp();
	std::vector<byte>& GetWrittenBmp();

private:

	std::vector<byte>		_UsedBmp;
	std::vector<byte>		_WrittenBmp;
	CDataMap						_DataMap;

	uint32	_TotalBlocks;
	uint32	_BlockSize;
	uint32	_BasePtr;
};

#endif
