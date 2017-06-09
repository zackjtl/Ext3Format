//---------------------------------------------------------------------------
#pragma hdrstop
#include "TestBlockManager.h"
#include "BlockManager.h"
#include "BaseError.h"
//---------------------------------------------------------------------------
/*
 * 	Auto allocate an amount of blocks, push the blocks into the input vector,
 * 	and returns the remaining blocks not allocated
 */
void TestAutoAllocBlock()
{
	CBlockManager	man(16384, 4096);

	vector<uint32>	blocks;

	uint32 ret = man.AutoAllocBlock(100, blocks);

	if ((blocks.size() != 100) || (ret != 0)) {
		throw CError(L"Not allocated the amount of indicated blocks #1");
	}

	ret = man.AutoAllocBlock(16384-100, blocks);

	if ((blocks.size() != 16384) || (ret != 0)) {
		throw CError(L"Not allocated the amount of indicated blocks #2");
	}

	ret = man.AutoAllocBlock(10, blocks);

	if ((blocks.size() != 16384) || (ret != 10)) {
		throw CError(L"The unused array is empty but still return non-empty for block request");
	}
}

/*
 * 	Auto allocate single block and return true or false
 */
void TestAutoAllocSingleBlock()
{
	CBlockManager	man(16384, 4096);

	vector<uint32>	blocks;

	uint32 singleBlock;
	/* The return of AutoAllocSingleBlock is true or false, not the remaining block */
	uint32 ret = man.AutoAllocSingleBlock(singleBlock);

	if ((singleBlock != 0) || (ret != 1)) {
		throw CError(L"Not allocated the amount of indicated blocks #1");
	}

	ret = man.AutoAllocBlock(16384, blocks);

	if ((blocks.size() != 16383) || (ret != 1)) {
		throw CError(L"Unexpected remaining blocks of request after single block allocation");
	}
}

/*
 * 	Occupy a specific block area.
 */
void TestOccupyBlock()
{
	CBlockManager	man(16384, 4096);

	vector<uint32>	blocks;

	uint32 ret = man.OccupyBlock(10, 3);

	if (ret != 1) {
		throw CError(L"Occupy block failed");
	}

	ret = man.OccupyBlock(11, 1);

	if (ret != 0) {
		throw CError(L"Not return failed when re-occupying a block");
	}

	ret = man.AutoAllocBlock(16384, blocks);

	if ((blocks.size() != 16381) || (ret != 3)) {
		throw CError(L"Unexpected remaining blocks fo request after occupying block");
	}
}

/*
 * 	Set Block Data to Occupied Blocks and Non-Occupied Blocks (need catch error)
 */
void TestSetBlockData()
{
	CBlockManager	man(16384, 4096);

	vector<uint32>	blocks;
	uint32 ret = man.OccupyBlock(10, 3);

	Bulk<byte>	dataBuffW(4096 * 4);
	Bulk<byte>	dataBuffR(4096 * 4);
	memset(dataBuffW, 0x55, 4096);
	memset(dataBuffW.Data() + 4096, 0xaa, 4096);
	memset(dataBuffW.Data() + 8192, 0x5a, 4096);
	memset(dataBuffW.Data() + 12288, 0xa5, 4096);

	blocks.push_back(11);
	blocks.push_back(12);
	blocks.push_back(13);

	try {
		man.SetBlockData(blocks, dataBuffW.Data(), dataBuffW.Size());
		throw CError(L"Not throw error while access to a non-allocated address");
	}
	catch (CError& Error) {
	}

	blocks.clear();

	blocks.push_back(10);
	blocks.push_back(11);
	blocks.push_back(12);

	try {
		man.SetBlockData(blocks, dataBuffW.Data(), 8192);
		throw CError(L"Not throw error while not input enough data buffer size");
	}
	catch (CError& Error) {
	}

	/* This time should be right */
	man.SetBlockData(blocks, dataBuffW.Data(), dataBuffW.Size());
}

/*
 * 	Get Block Data from Occupied Blocks and Non-Occupied Blocks (need catch error)
 */
void TestGetBlockData()
{
	CBlockManager	man(16384, 4096);

	vector<uint32>	blocks;
	uint32 ret = man.OccupyBlock(10, 3);

	Bulk<byte>	dataBuffW(4096 * 4);
	Bulk<byte>	dataBuffR(4096 * 4);
	memset(dataBuffW, 0x55, 4096);
	memset(dataBuffW.Data() + 4096, 0xaa, 4096);
	memset(dataBuffW.Data() + 8192, 0x5a, 4096);
	memset(dataBuffW.Data() + 12288, 0xa5, 4096);

	blocks.push_back(10);
	blocks.push_back(11);

	man.SetBlockData(blocks, dataBuffW.Data(), dataBuffW.Size());

	blocks.clear();
	blocks.push_back(10);
	blocks.push_back(11);
	blocks.push_back(12);

	try {
		man.GetBlockData(blocks, dataBuffR.Data(), 4096 * 3);
		throw CError(L"Not throw error while access to a non-written address");
	}
	catch (CError& Error) {
	}
	blocks.clear();
	blocks.push_back(12);
 	man.SetBlockData(blocks, dataBuffW.Data() + 8192, 4096);

	blocks.clear();
	blocks.push_back(11);
	blocks.push_back(12);
	blocks.push_back(13);

	try {
		man.GetBlockData(blocks, dataBuffR.Data(), 4096 * 3);
		throw CError(L"Not throw error while access to a non-occupied address");
	}
	catch (CError& Error) {
	}

	blocks.clear();
	blocks.push_back(10);
	blocks.push_back(11);
	blocks.push_back(12);

	try {
		man.GetBlockData(blocks, dataBuffR.Data(), dataBuffR.Size());
		throw CError(L"Not throw error while request exceeding data length");
	}
	catch (CError& Error) {
	}

	/* This time should be right */
	man.GetBlockData(blocks, dataBuffR.Data(), 4096 * 3);

	if (memcmp(dataBuffR, dataBuffW, 4096 * 3) != 0) {
		throw CError(L"Pattern miscompared between setting and getting data");
	}
}

/*
 * 	Set/Get Single Block Data
 */
void TestSetAndGetSingleBlockData()
{
	CBlockManager	man(16384, 4096);

	vector<uint32>	blocks;
	uint32 ret = man.OccupyBlock(10, 3);

	Bulk<byte>	dataBuffW(4096 * 4);
	Bulk<byte>	dataBuffR(4096 * 4);
	memset(dataBuffW, 0x55, 4096);
	memset(dataBuffW.Data() + 4096, 0xaa, 4096);
	memset(dataBuffW.Data() + 8192, 0x5a, 4096);
	memset(dataBuffW.Data() + 12288, 0xa5, 4096);

	try {
		man.SetSingleBlockData(9, dataBuffW, 4096);
		throw CError(L"Not throw error when setting data to a non-occupied single block");
	}
	catch (CError& Error) {
	}
	man.SetSingleBlockData(10, dataBuffW, 4096);

	try {
		man.GetSingleBlockData(9, dataBuffR, 4096);
		throw CError(L"Not throw error when getting data from a non-occupied single block");
	}
	catch (CError& Error) {
	}
	/* Test GetSingleBlockDataBuffer function */
	try {
		Bulk<byte>* ret = man.GetSingleBlockDataBuffer(9);
		throw CError(L"Not throw error when getting data from a non-occupied single block");
	}
	catch (CError& Error) {
	}


	try {
		man.GetSingleBlockData(11, dataBuffR, 4096);
		throw CError(L"Not throw error when getting data from a non-written single block");
	}
	catch (CError& Error) {
	}
	/* Test GetSingleBlockDataBuffer function */
	try {
		Bulk<byte>* ret = man.GetSingleBlockDataBuffer(11);
		throw CError(L"Not throw error when getting data from a non-writeen single block");
	}
	catch (CError& Error) {
	}

	/* This time should be right */
	man.GetSingleBlockData(10, dataBuffR, 4096);
	Bulk<byte>* retBulk = man.GetSingleBlockDataBuffer(10);

	if (retBulk == NULL) {
		throw CError(L"Unexpected return null bulk pointer to a written block");
	}
	if (memcmp(retBulk->Data(), dataBuffW.Data(), 4096) != 0) {
		throw CError(L"Pattern miscompared between setting and getting data "
								 L"(by returning bulk ponter)");
	}

	man.SetSingleBlockData(11, dataBuffW.Data() + 4096, 4096);
	man.SetSingleBlockData(12, dataBuffW.Data() + 8192, 4096);

	blocks.push_back(10);
	blocks.push_back(11);
	blocks.push_back(12);

	man.GetBlockData(blocks, dataBuffR.Data(), 4096 * 3);

	if (memcmp(dataBuffR, dataBuffW, 4096 * 3) != 0) {
		throw CError(L"Pattern miscompared between setting and getting data");
	}
}

/*
 * 	Block Overwrite Test
 */
void TestBlockOverwrite()
{
	CBlockManager	man(16384, 4096);

	vector<uint32>	blocks;
	uint32 ret = man.OccupyBlock(10, 3);

	Bulk<byte>	dataBuffW(4096 * 4);
	Bulk<byte>	dataBuffR(4096 * 4);
	memset(dataBuffW, 0x55, 4096);
	memset(dataBuffW.Data() + 4096, 0xaa, 4096);
	memset(dataBuffW.Data() + 8192, 0x5a, 4096);
	memset(dataBuffW.Data() + 12288, 0xa5, 4096);

	blocks.push_back(10);
	blocks.push_back(11);
	blocks.push_back(12);

	man.SetBlockData(blocks, dataBuffW, 3 * 4096);

	memset(dataBuffW.Data() + 8192, 0x33, 4096);
	man.SetSingleBlockData(12, dataBuffW.Data() + 8192, 4096);

	man.GetBlockData(blocks, dataBuffR.Data(), 4096 * 3);

	if (memcmp(dataBuffR, dataBuffW, 4096 * 3) != 0) {
		throw CError(L"Pattern miscompared after overwritten data");
	}

	man.OccupyBlock(13, 1);

	memset(dataBuffW.Data() + 8192, 0x99, 4096);

	blocks.clear();
	blocks.push_back(11);
	blocks.push_back(12);
	blocks.push_back(13);
	man.SetBlockData(blocks, dataBuffW.Data() + 4096, 4096 * 3);

	blocks.push_back(10);
	sort(blocks.begin(), blocks.end());

	man.GetBlockData(blocks, dataBuffR.Data(), 4096 * 4);

	if (memcmp(dataBuffR, dataBuffW, 4096 * 4) != 0) {
		throw CError(L"Pattern miscompared after overwritten data");
	}
}
