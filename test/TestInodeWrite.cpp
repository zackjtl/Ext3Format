//---------------------------------------------------------------------------
#pragma hdrstop
#include "TestInodeWrite.h"
#include "Inode.h"
#include "BlockManager.h"
#include "Tester.h"
#include <cstring>

const uint32 TotalBlock = 33554432;
const uint32 BlockSize = 1024;
const uint32 HalfBlock = BlockSize / 2;
const uint32 AddrPerBlock = BlockSize / sizeof(uint32); // 256

CBlockManager man(TotalBlock, BlockSize);
CInode inode(0, BlockSize);

uint32 DirectBlocks = EXT2_NDIR_BLOCKS;
uint32 IndirectBlocks = AddrPerBlock;
uint32 DIndirectBlocks = AddrPerBlock * AddrPerBlock;
uint32 TIndirectBlocks = 2 * AddrPerBlock * AddrPerBlock;

uint32 TargetTotalBlocks = DirectBlocks + IndirectBlocks	+
													 DIndirectBlocks + TIndirectBlocks;

Bulk<byte>	Buffer(TargetTotalBlocks * BlockSize);

uint32 BlockPos = 0;

/*
 *  Program rule: Do not count half block to the written blocks until
 *								the another half has been written.
 */

#define check_leaf_cnt(x) {assert_eq(inode.CalcLeafCount(), x); }

void GetRealBlocks(vector<uint32>& Blocks, uint32 StartBlock, uint32 Count)
{
	for (uint32 i = StartBlock; i < (StartBlock + Count); ++i) {
		uint32 real;

		if (!inode.IndexToRealBlock(i, real)) {
			throw CError(L"Cannot find the indicated index to the real block address");
		}
		Blocks.push_back(real);
	}
}

/*
 * 	Write inode direct blocks cross to the indirect blocks
 */
void WriteDirectBlockToIndirectBlock()
{
	BlockPos = 0;

	vector<uint32>	real_blocks;

	make_pattern(Buffer.Data(), 0x00, Buffer.Size());

	uint32 new_blocks = 0;
	uint32 blockCnt = DirectBlocks / 2 - 1;
	uint32 length = blockCnt * BlockSize + HalfBlock;
	byte* ptr = Buffer.Data();

	inode.SetData(man, ptr, length);

	ptr += length;
	BlockPos += blockCnt;
	new_blocks += (blockCnt + 1);

	check_leaf_cnt(new_blocks);

	/* Change the last half block pattern */
	make_pattern(ptr, 0x00, HalfBlock);

	blockCnt = ((DirectBlocks / 2) + 1) + (IndirectBlocks / 2 - 1);
	length = blockCnt * BlockSize + HalfBlock + HalfBlock;

	inode.SetData(man, ptr, length);

	ptr += length;
	BlockPos += (blockCnt + 1);
	new_blocks += (blockCnt + 1);

	check_leaf_cnt(new_blocks);

	GetRealBlocks(real_blocks, 0, new_blocks);

	Bulk<byte>	readBuff(new_blocks * BlockSize);

	man.GetBlockData(real_blocks, readBuff.Data(), readBuff.Size());

	assert_eq(readBuff.Size() - HalfBlock, inode.GetSize());

	if (memcmp(readBuff.Data(), Buffer.Data(), readBuff.Size() - HalfBlock) != 0) {
		throw CError(L"Compare with inode written data failed");
	}
}


/*
 * 	Write inode indirect blocks cross to the d-indirect blocks
 */
void WriteIndirectBlockToDIndirectBlock()
{
	uint32 StartPos = BlockPos;

	vector<uint32>	real_blocks;

	uint32 new_blocks = 0;
	uint32 blockCnt = (IndirectBlocks / 2 + 1) + (DIndirectBlocks / 2 - 1);
	uint32 length = blockCnt * BlockSize;
	byte* ptr = Buffer.Data() + StartPos * BlockSize - HalfBlock;

	make_pattern(ptr, 0x00, HalfBlock);

	inode.SetData(man, ptr, length);

	BlockPos += blockCnt;
	new_blocks += (blockCnt);

	check_leaf_cnt(BlockPos+1);

	GetRealBlocks(real_blocks, StartPos, (new_blocks + 1));

	Bulk<byte>	readBuff((new_blocks + 1) * BlockSize);

	man.GetBlockData(real_blocks, readBuff.Data(), readBuff.Size());

	assert_eq((BlockPos + 1) * BlockSize - HalfBlock, inode.GetSize());

	/* No compare for the  real half and the last half in thr readed area */
	if (memcmp(readBuff.Data() + HalfBlock, ptr, readBuff.Size() - 2 * HalfBlock) != 0) {
		throw CError(L"Compare with inode written data failed");
	}
	ptr += length;
}


/*
 * 	Write inode d-indirect blocks cross to the t-indirect blocks
 */
void WriteDIndirectBlockToTIndirectBlock()
{
	uint32 StartPos = BlockPos;

	vector<uint32>	real_blocks;

	uint32 new_blocks = 0;
	uint32 blockCnt = (DIndirectBlocks / 2 + 1) + (TIndirectBlocks / 2 - 1);
	uint32 length = blockCnt * BlockSize;
	byte* ptr = Buffer.Data() + StartPos * BlockSize - HalfBlock;

	make_pattern(ptr, 0x00, HalfBlock);

	inode.SetData(man, ptr, length);

	BlockPos += blockCnt;
	new_blocks += (blockCnt);

	check_leaf_cnt(BlockPos+1);

	GetRealBlocks(real_blocks, StartPos, (new_blocks + 1));

	Bulk<byte>	readBuff((new_blocks + 1) * BlockSize);

	man.GetBlockData(real_blocks, readBuff.Data(), readBuff.Size());

	assert_eq((BlockPos + 1) * BlockSize - HalfBlock, inode.GetSize());

	/* No compare for the  real half and the last half in thr readed area */
	if (memcmp(readBuff.Data() + HalfBlock, ptr, readBuff.Size() - 2 * HalfBlock) != 0) {
		throw CError(L"Compare with inode written data failed");
	}
	ptr += length;
}

void ValidateIndirectTable()
{
	inode.ValidateIndirectLink(man);
}

void ValidateDIndirectTable()
{
	inode.ValidateMultilayerLink(man, inode.DIndirect);
}

void ValidateTIndirectTable()
{
	inode.ValidateMultilayerLink(man, inode.TIndirect);
}

/*
 * 	An universal used function for testing. Create data with continue
 *  patterns in every 512 bytes.
 */
void make_pattern(byte* Buffer, byte StartPattern, uint32 Length)
{
	uint32 sectorCnt = Length / 512;
	uint32 redundant = Length % 512;
	uint16* ptr;

	srand(time(NULL));

	for (uint32 i = 0; i < sectorCnt; ++i) {
		if (StartPattern == 255) {
			StartPattern = 0;
		}
		ptr = (uint16*)&Buffer[i*512];

		*ptr++ = rand();
		*ptr++ = rand();

		memset((byte*)ptr, StartPattern++, 508);
	}
	if (StartPattern == 255) {
		StartPattern = 0;
	}
	if (redundant >= 4) {
		ptr = (uint16*)&Buffer[sectorCnt * 512];
		*ptr++ = rand();
		*ptr++ = rand();

		redundant -= 4;
	}
	if (redundant)
		memset((byte*)ptr, StartPattern, redundant);
}


