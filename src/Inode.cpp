#include "Inode.h"
#include "GlobalDef.h"
#include "BaseError.h"

CInode* CInode::Create(uint16 Mode, uint32 Block_Size)
{
	// TODO: Return a new CInode object with the type specified by the input Mode.
}

CInode::CInode(uint16 Mode, uint32 Block_Size)
	: Mode(Mode),
		BlockSize(Block_Size),
		AddrPerBlock(Block_Size / sizeof(uint32)),
		IndirectBlockThreshold(EXT2_NDIR_BLOCKS),
		DIndirectBlockThreshold(EXT2_NDIR_BLOCKS + AddrPerBlock),
		TIndirectBlockThreshold(DIndirectBlockThreshold + AddrPerBlock * AddrPerBlock)
{
	_Position = 0;
	_Size = 0;
}

CInode::~CInode()
{

}

uint64 CInode::GetSize()
{
	return _Size;
}
uint64 CInode::GetPosition()
{
	return _Position;
}

#define get_parent(x) (x / AddrPerBlock)
#define to_local(x)   (x % AddrPerBlock)

/*
 *  Set data into block units in the block manager.
 *  But not real write into storage media.
 */
int CInode::WriteData(CBlockManager& BlockMan, byte* Buffer, int Length)
{
	uint32 blockPos = _Position / BlockSize;
	uint32 headOffset = _Position % BlockSize;
	uint32 remainBytes = Length;
	uint32 bytesWritten = 0;
	uint32 realBlock;

	byte* ptr = Buffer;

	/* The data position is in the latest final block that old written */
	if ((headOffset > 0) && (headOffset < BlockSize)) {
		if (!IndexToRealBlock(blockPos, realBlock)) {
			if (alloc_blocks(BlockMan, 1) != 0) {
				throw CError(L"Insufficient block to allocate or the inode is full");
			}
		}

    bytesWritten = BlockSize - headOffset;
    Bulk<byte> buffer(BlockSize);

    BlockMan.GetSingleBlockData(realBlock, buffer, buffer.Size());
    memcpy(buffer.Data() + headOffset, Buffer, bytesWritten);
    BlockMan.SetSingleBlockData(realBlock, buffer, buffer.Size());

    ptr += bytesWritten;
    remainBytes -= bytesWritten;

		_Position += bytesWritten;
		_Size += bytesWritten;
    ++blockPos;
  }

  /* Set data into middle block */
  uint32 middleBlocks = remainBytes / BlockSize;
  uint32 redundantBlock = remainBytes % BlockSize > 0 ? 1 : 0;

	if (alloc_blocks(BlockMan, middleBlocks + redundantBlock) != 0) {
 		throw CError(L"Insufficient block to allocate or the inode is full");
	}

  for (uint32 block = 0; block < middleBlocks; ++block) {
		if (!IndexToRealBlock(blockPos, realBlock)) {
			throw CError(L"Unexpected response of IndexToReadBlock function");
		}
    BlockMan.SetSingleBlockData(realBlock, ptr, BlockSize);

    ptr += BlockSize;
    remainBytes -= BlockSize;
		_Position += BlockSize;
		_Size += BlockSize;
    ++blockPos;
	}

	/* Set redundant data into the final block */
	if (remainBytes) {
		assert(redundantBlock == 1);

		if (!IndexToRealBlock(blockPos, realBlock)) {
			throw CError(L"Unexpected response of IndexToReadBlock function");
		}
		Bulk<byte> tailBuffer(BlockSize);
		memset(tailBuffer, 0x00, BlockSize);
		memcpy(tailBuffer.Data(), ptr, remainBytes);
		BlockMan.SetSingleBlockData(realBlock, tailBuffer, tailBuffer.Size());
		_Position += remainBytes;
		_Size += remainBytes;
		remainBytes = 0;
	}
	assert(remainBytes == 0);

  return Length;
}


uint32 CInode::alloc_blocks(CBlockManager& BlockMan, uint32 RequireBlocks)
{
	uint32 remain = alloc_dir_blocks(BlockMan, RequireBlocks);

	if (remain) {
		remain = alloc_indr_blocks(BlockMan, remain);

		if (remain) {
			remain = alloc_dindr_blocks(BlockMan, remain);

			if (remain) {
				remain = alloc_tindr_blocks(BlockMan, remain);
			}
		}
	}
	return remain;
}
/*
 *	Allocate direct addressing blocks
 */
uint32 CInode::alloc_dir_blocks(CBlockManager& BlockMan, uint32 RequireBlocks)
{
	uint32 remain = RequireBlocks;
	uint32 dir_offset = Direct.size();
	uint32 dir_remain = EXT2_NDIR_BLOCKS - dir_offset;
	uint32 direct_alloc_cnt = min_of(remain, dir_remain);

	if (direct_alloc_cnt != 0) {
		for (int i = 0; i < direct_alloc_cnt; ++i) {
			uint32 block;

			if (BlockMan.AutoAllocSingleBlock(block)) {
				Inode.Blocks[dir_offset + i] = block;
				Direct.push_back(block);
			}
			else {
				throw CError(L"Out of block used");
			}
			--remain;
		}
	}
	return (RequireBlocks - direct_alloc_cnt);
}
/*
 *	Allocate indirect addressing blocks
 */
uint32 CInode::alloc_indr_blocks(CBlockManager& BlockMan, uint32 RequireBlocks)
{
	uint32 indir_offset = Indirect.size();
	uint32 indir_remain = AddrPerBlock - indir_offset;
  uint32 request_cnt = min_of(indir_remain, RequireBlocks);

	if (indir_offset == 0) {
		uint32 indir_addr;
		if (BlockMan.AutoAllocSingleBlock(indir_addr) == false) {
			throw CError(L"Out of block used");
		}
		Inode.Blocks[EXT2_IND_BLOCK] = indir_addr;
		BlockMan.CreateSingleBlockDataBuffer(indir_addr);
	}

	uint remain = BlockMan.AutoAllocBlock(request_cnt, Indirect);

	if (remain) {
  	throw CError(L"Out of block used");
	}

	/* Update the indirect block's address table */
	uint32 total = Indirect.size();
	uint32 position = total - request_cnt;
	Bulk<byte>* buffer = BlockMan.GetSingleBlockDataBuffer(
																	Inode.Blocks[EXT2_IND_BLOCK]);

	uint32* ptr = (uint32*)buffer->Data();
	ptr += position;

	for ( ; position < total; ++position) {
		*ptr++ = Indirect[position];
	}
	return (RequireBlocks - request_cnt);
}
/*
 *	Allocate double-indirec addressing blocks
 */
uint32 CInode::alloc_dindr_blocks(CBlockManager& BlockMan, uint32 RequireBlocks)
{
	if (DIndirect.size() == 0) {
		DIndirect.resize(2);
		uint32 dindir;
		if (BlockMan.AutoAllocSingleBlock(dindir) == false) {
			throw CError(L"Out of block used");
		}
		Inode.Blocks[EXT2_DIND_BLOCK] = dindir;
    BlockMan.CreateSingleBlockDataBuffer(dindir);
	}
	uint remain = GrowthTree(DIndirect, BlockMan, RequireBlocks);
	return remain;
}
/*
 *	Allocate tripple-indirect addressing blocks
 */
uint32 CInode::alloc_tindr_blocks(CBlockManager& BlockMan, uint32 RequireBlocks)
{
	if (TIndirect.size() == 0) {
		TIndirect.resize(3);
		uint32 tindir;
		if (BlockMan.AutoAllocSingleBlock(tindir) == false) {
			throw CError(L"Out of block used");
		}
		Inode.Blocks[EXT2_TIND_BLOCK] = tindir;
    BlockMan.CreateSingleBlockDataBuffer(tindir);
	}
	uint remain = GrowthTree(TIndirect, BlockMan, RequireBlocks);
	return remain;
}

/*
 * 	The general allocating function for indirect matrix
 */
uint32 CInode::GrowthTree(CIndrMatrix& Matrix, CBlockManager& BlockMan, uint32 RequireBlocks)
{
	uint32 layer_curr_size[3];
	uint32 layer_max_size[3];
	uint32 layer_new_size[3];
	uint32 layer_alloc_size[3];
	uint32 layer_remain_not_alloc[3];

	memset(layer_new_size, 0, 3 * sizeof(uint));

	make_layer_curr_sizes(layer_curr_size, Matrix);
	make_layer_max_sizes(layer_max_size);

	uint layer_cnt = Matrix.size();
	uint leaf_layer = layer_cnt - 1;

	uint max_leaf_cnt = layer_max_size[leaf_layer];
	uint curr_leaf_cnt = layer_curr_size[leaf_layer];
	uint remain_leaf_cnt = max_leaf_cnt - curr_leaf_cnt;

	uint alloc_leaf_cnt = min_of(remain_leaf_cnt, RequireBlocks);

	layer_alloc_size[leaf_layer] = alloc_leaf_cnt;
	layer_new_size[leaf_layer] = curr_leaf_cnt + alloc_leaf_cnt;

	if (leaf_layer > 0) {
		for (int layer = (leaf_layer - 1); layer >= 0; --layer) {
			uint remain_cnt = layer_max_size[layer] - layer_curr_size[layer];
			uint child_cnt = layer_new_size[layer + 1];
			uint need_cnt = div_ceil(child_cnt, AddrPerBlock);

			/* The required size has been checked for leaf, so it is impossible to
					require exceed parent's capacity. */
			assert(need_cnt <= layer_max_size[layer]);

			layer_new_size[layer] = need_cnt;
			layer_alloc_size[layer] = need_cnt - layer_curr_size[layer];
		}
	}

	for (uint8 layer = 0; layer <= leaf_layer; ++layer) {
		vector<uint32> retBlocks;

		uint32 remain = BlockMan.AutoAllocBlock(layer_alloc_size[layer], retBlocks);

		Matrix[layer].insert(Matrix[layer].end(), retBlocks.begin(), retBlocks.end());

		if (layer == leaf_layer) {
			if (remain)
				throw CError(L"Out of block used");

			return (RequireBlocks - layer_alloc_size[layer]);
		}
		else {
			assert(remain == 0);
		}
		if (layer > 0) {
			UpdateAddressTable(Matrix, BlockMan, retBlocks.size(), layer);
		}
	}
}

/*
 * 	Update the address table data of the new adding blocks' parents.
 */
void CInode::UpdateAddressTable(CIndrMatrix& Matrix, CBlockManager& BlockMan,
															 uint32 NewCount, uint8 Layer)
{
	assert(Layer > 0);
	uint8 parent_layer = Layer - 1;

	vector<uint32>& parent_array = Matrix[parent_layer];
	vector<uint32>& child_array = Matrix[Layer];
	uint child_total = child_array.size();

	uint32 position = child_total - NewCount;
	uint32 local_pos = to_local(position);
	uint32 parent_block = get_parent(position);

	Bulk<byte>* buffer = BlockMan.GetSingleBlockDataBuffer(parent_block);

  if (buffer == NULL) {
    buffer = BlockMan.CreateSingleBlockDataBuffer(parent_block);
  }  
	uint32* ptr = (uint32*)buffer->Data();
	ptr += local_pos;

	while (position < child_total) {
		*ptr++ = child_array[position++];
    
		if (get_parent(position) != parent_block) {
      parent_block = get_parent(position);
			buffer = BlockMan.GetSingleBlockDataBuffer(parent_block);      

      if (buffer == NULL) {
        buffer = BlockMan.CreateSingleBlockDataBuffer(parent_block);
      }        
      ptr = (uint32*)buffer->Data();
		}
	}

}

void CInode::make_layer_curr_sizes(uint32 Sizes[3], CIndrMatrix& Matrix)
{
	uint layer_cnt = Matrix.size();

	for (int i = 0; i < 3; ++i) {
		if (i < layer_cnt)
			Sizes[i] = Matrix[i].size();
		else
			Sizes[i] = 0;
	}
}

void CInode::make_layer_max_sizes(uint32 Sizes[3])
{
	for (int i = 0; i < 3; ++i) {
		uint32 max = AddrPerBlock;

		for (int f = 0; f < i; ++f) {
			max *= AddrPerBlock;
		}
		Sizes[i] = max;
	}
}

bool CInode::IndexToRealBlock(uint32 Index, uint32& Real)
{
  if (Index < IndirectBlockThreshold) {
    Real = Inode.Blocks[Index];
  }
  else if (Index >= IndirectBlockThreshold &&
           Index < DIndirectBlockThreshold) {           
		if (Indirect.size() <= (Index - IndirectBlockThreshold))
			return false;
    Real = Indirect[Index - IndirectBlockThreshold];
  }
  else if (Index >= DIndirectBlockThreshold &&
           Index < TIndirectBlockThreshold) {           
    uint32 offset = Index - DIndirectBlockThreshold;
    
		uint32 layer0 = offset / AddrPerBlock;
		if (DIndirect.size() != 2)
			return false;
			////throw CError(L"Double Indirect block array not created completed");
		if (DIndirect[0].size() <= layer0)
      return false;
			////throw CError(L"Double Indirect block root array not created completed");
		if (DIndirect[1].size() <= offset)
			return false;
			////throw CError(L"Double Indirect block leaf array not created completed");

		Real = DIndirect[1][offset];
  }          
  else {
    uint32 offset = Index - TIndirectBlockThreshold;
		uint32 layer1 = offset / AddrPerBlock;
		uint32 layer0 = layer1 / AddrPerBlock;
		if (TIndirect.size() != 3)
			return false;
			////throw CError(L"Tripple Indirect block array not created completed");
		if (TIndirect[0].size() <= (layer0))
			return false;
			////throw CError(L"Tripple Indirect block root array not created completed");
		if (TIndirect[1].size() <= (layer1))
			return false;
			////throw CError(L"Tripple Indirect block layer1 array not created completed");
		if (TIndirect[2].size() <= offset) {
			return false;
			////throw CError(L"Tripple Indirect block leaf array not created completed");
    }
    Real = TIndirect[2][offset];
	}
	return true;
}

 uint32 CInode::CalcLeafCount()
{
  uint32 count = 0;

  count += Direct.size();
  count += Indirect.size();

	if (DIndirect.size() == 2)
    count += DIndirect[1].size();
  if (TIndirect.size() == 3)
		count += TIndirect[2].size();

	return count;
}

void CInode::GetLeafBlocks(vector<uint32>& Blocks)
{
	append_vect(Blocks, Direct);
	append_vect(Blocks, Indirect);

	if (DIndirect.size() == 2) {
		append_vect(Blocks, DIndirect[1]);
	}
	if (TIndirect.size() == 3) {
		append_vect(Blocks, TIndirect[2]);
	}
}



