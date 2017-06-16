//---------------------------------------------------------------------------
#pragma hdrstop
#include "TestInode.h"
#include "Inode.h"
#include "BlockManager.h"
#include "Tester.h"
#include <cstring>
//---------------------------------------------------------------------------
/*
 * 	Test allocate direct blocks
 */
void TestAllocDirBlocks()
{
	/* Suppose the block size is only 256 bytes */
	CBlockManager man(16384, 64 * sizeof(uint32));
	CInode inode(0, 64 * sizeof(uint32));

	memset(inode.Inode.Blocks, 0xFF, EXT2_N_BLOCKS * sizeof(inode.Inode.Blocks[0]));

	uint32 remain = inode.alloc_dir_blocks(man, 1);
	assert_eq(remain, 0);

	remain = inode.alloc_dir_blocks(man, EXT2_NDIR_BLOCKS - 1);
	assert_eq(remain, 0);

	remain = inode.alloc_dir_blocks(man, 2);
	assert_eq(remain, 2);

	for (int i = 0; i < EXT2_NDIR_BLOCKS; ++i) {
		assert_neq(inode.Inode.Blocks[i], 0xFFFFFFFF);
	}
	assert_eq(inode.Direct.size(), EXT2_NDIR_BLOCKS);
}

/*
 * 	Test allocate indirect blocks. The expected blocks should be allocated
 *	is the max. address number contained in a block. (ex: 4096 / 4)
 */
void TestAllocIndirBlocks()
{
	CBlockManager man(16384, 64 * sizeof(uint32));
	CInode inode(0, 64 * sizeof(uint32));

	memset(inode.Inode.Blocks, 0xFF, EXT2_N_BLOCKS * sizeof(inode.Inode.Blocks[0]));
	assert_eq(inode.Inode.Blocks[EXT2_IND_BLOCK], 0xFFFFFFFF);

	uint32 expectCnt = 64;
	uint32 remain = inode.alloc_indr_blocks(man, 2);

	assert_neq(inode.Inode.Blocks[EXT2_IND_BLOCK], 0xFFFFFFFF);
	assert_eq(remain, 0);

	remain = inode.alloc_indr_blocks(man, expectCnt - 2);
	assert_eq(remain , 0);

	remain = inode.alloc_indr_blocks(man, 2);
	assert_eq(remain, 2);
}

/*
 * 	Test allocate double-indirect blocks. The expected blocks should be allocated
 *	N * N, for which N is the max. address number contained in a block.
 *  In addition, N blocks should be callocated for the first layer.
 */
void TestAllocDIndirBlocks()
{
	CBlockManager man(16384, 64 * sizeof(uint32));
	CInode inode(0, 64 * sizeof(uint32));

	memset(inode.Inode.Blocks, 0xFF, EXT2_N_BLOCKS * sizeof(inode.Inode.Blocks[0]));

	uint32 expectCnt = 64 * 64;
	uint32 remain = inode.alloc_dindr_blocks(man, 2);

	assert_neq(inode.Inode.Blocks[EXT2_DIND_BLOCK], 0xFFFFFFFF);
	assert_eq(remain, 0);

	assert_eq(inode.DIndirect[0].size(), 1);
	assert_eq(inode.DIndirect[1].size(), 2);

	remain = inode.alloc_dindr_blocks(man, 64);

	assert_eq(remain, 0);
	assert_eq(inode.DIndirect[0].size(), 2);
	assert_eq(inode.DIndirect[1].size(), 66);

	remain = inode.alloc_dindr_blocks(man, expectCnt - 66 + 2);
	assert_eq(remain, 2);
	assert_eq(inode.DIndirect[0].size(), 64);
	assert_eq(inode.DIndirect[1].size(), expectCnt);
}

/*
 * 	Test allocate double-indirect blocks. The expected blocks should be allocated
 *	N * N, for which N is the max. address number contained in a block.
 *  In addition, N blocks should be callocated for the first layer.
 */
void TestAllocTIndirBlocks()
{
	CBlockManager man(2097152, 64 * sizeof(uint32));
	CInode inode(0, 64 * sizeof(uint32));

	memset(inode.Inode.Blocks, 0xFF, EXT2_N_BLOCKS * sizeof(inode.Inode.Blocks[0]));
	assert_eq(inode.Inode.Blocks[EXT2_TIND_BLOCK], 0xFFFFFFFF);

	uint32 expectCnt = 64 * 64 * 64;
	uint32 remain = inode.alloc_tindr_blocks(man, 2);

	assert_neq(inode.Inode.Blocks[EXT2_TIND_BLOCK], 0xFFFFFFFF);
	assert_eq(remain, 0);

	assert_eq(inode.TIndirect[0].size(), 1);
	assert_eq(inode.TIndirect[1].size(), 1);
	assert_eq(inode.TIndirect[2].size(), 2);

	remain = inode.alloc_tindr_blocks(man, 64);

	assert_eq(remain, 0);
	assert_eq(inode.TIndirect[0].size(), 1);
	assert_eq(inode.TIndirect[1].size(), 2);
	assert_eq(inode.TIndirect[2].size(), 64 + 2);

	remain = inode.alloc_tindr_blocks(man, 64 * 64);

	assert_eq(remain, 0);
	assert_eq(inode.TIndirect[0].size(), 2);
	assert_eq(inode.TIndirect[1].size(), 64 + 2);
	assert_eq(inode.TIndirect[2].size(), 64 + 2 + 64*64);

	remain = inode.alloc_tindr_blocks(man, 64 * 64 * 64);

	assert_eq(remain, 64 + 2 + 64*64);
	assert_eq(inode.TIndirect[0].size(), 64);
	assert_eq(inode.TIndirect[1].size(), 64 * 64);
	assert_eq(inode.TIndirect[2].size(), 64 * 64 * 64);
}
