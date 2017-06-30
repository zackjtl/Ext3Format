#include "Inode.h"
#include "e3fs_def.h"
#include "BaseError.h"
#include "FileInode.h"
#include "FolderInode.h"
#include "e3fs_uuid.h"
#include "TypeConv.h"
#include <cassert>
#include <time.h>
#include <cstring>

CInode* CInode::Create(uint32 Type, uint16 BlockSize)
{
	// TODO: Return a new CInode object with the type specified by the input Type.
	if (LINUX_S_IFDIR == Type) {
		return new CFolderInode(BlockSize);
	}
	else if (LINUX_S_IFREG == Type) {
 		return new CFileInode(BlockSize);
	}
  else if (0 == Type) {
    return new CInode(0, BlockSize);
  }
	else {
		throw CError(L"Attempt to create a unknown type of inode failed");
	}
}

CInode::CInode(uint32 Type, uint16 BlockSize)
	: Type(Type),
		_BlockSize(BlockSize),
		_AddrPerBlock(BlockSize / sizeof(uint32)),
		IndirectBlockThreshold(EXT2_NDIR_BLOCKS),
		DIndirectBlockThreshold(EXT2_NDIR_BLOCKS + _AddrPerBlock),
		TIndirectBlockThreshold(DIndirectBlockThreshold + _AddrPerBlock * _AddrPerBlock),
		_Index(0),
		_GroupID(0),
		Permissions(000),
		_Name("")
{
  memset((byte*)&Inode, 0x00, sizeof(TInode));
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

uint32 CInode::GetIndex()
{
	return _Index;
}

string CInode::GetName()
{
	return _Name;
}

void CInode::SetIndex(uint32 Index)
{
	_Index = Index;
}

void CInode::SetGroupID(uint16 GroupID)
{
  _GroupID = GroupID;
}

void CInode::SetName(const string& Name)
{
	_Name = Name;
}

void CInode::SetPermissions(uint16 Value)
{
  Permissions = Value;
}

void CInode::UpdateInodeTable()
{
  Inode.Mode = OctToDec(Type + Permissions);
  Inode.Uid = 0;
  Inode.SizeInBytesLo = (uint32)_Size;
  Inode.SizeInBytesHi = _Size >> 32;
  Inode.AccessTime = GetPosixTime();
  Inode.InodeChangeTime = Inode.AccessTime;
  Inode.ModificationTime = Inode.AccessTime;
  Inode.DeleteTime = 0;
  Inode.GroupId = _GroupID;
  Inode.SectorCount = CalculateSectorCount(_Size);
  Inode.FileFlags = 0;
  Inode.HardLinkCnt = Type == LINUX_S_IFREG ? 1 : 0;
  Inode.OS_Dep1 = 0;
  Inode.FileVersion = 0;
  Inode.FragAddress = 0;
  Inode.FragNumber = 0;
  Inode.FragSize = 0;
  Inode.Padding = 0;
  memset((byte*)&Inode.Rsvd[0], 0, sizeof(Inode.Rsvd));  
}

/*
 *  Set data into block units in the block manager.
 *  But not real write into storage media.
 */
int CInode::SetData(CBlockManager& BlockMan, byte* Buffer, uint32 Length)
{
	uint32 blockPos = _Position / _BlockSize;
	uint32 headOffset = _Position % _BlockSize;
	uint32 remainBytes = Length;
	uint32 bytesWritten = 0;
	uint32 realBlock;

	byte* ptr = Buffer;

	/* The data position is in the latest final block that old written */
	if ((headOffset > 0) && (headOffset < _BlockSize)) {
		if (!IndexToRealBlock(blockPos, realBlock)) {
			if (alloc_blocks(BlockMan, 1) != 0) {
				throw CError(L"Insufficient block to allocate or the inode is full");
			}
		}

		bytesWritten = Length >= _BlockSize ? (_BlockSize - headOffset) : Length;
		Bulk<byte> buffer(_BlockSize);

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
  uint32 middleBlocks = remainBytes / _BlockSize;
  uint32 redundantBlock = remainBytes % _BlockSize > 0 ? 1 : 0;

	if (alloc_blocks(BlockMan, middleBlocks + redundantBlock) != 0) {
 		throw CError(L"Insufficient block to allocate or the inode is full");
	}

  for (uint32 block = 0; block < middleBlocks; ++block) {
		if (!IndexToRealBlock(blockPos, realBlock)) {
			throw CError(L"Unexpected response of IndexToReadBlock function");
		}
    BlockMan.SetSingleBlockData(realBlock, ptr, _BlockSize);

    ptr += _BlockSize;
    remainBytes -= _BlockSize;
		_Position += _BlockSize;
		_Size += _BlockSize;
    ++blockPos;
	}

	/* Set redundant data into the final block */
	if (remainBytes) {
		assert(redundantBlock == 1);

		if (!IndexToRealBlock(blockPos, realBlock)) {
			throw CError(L"Unexpected response of IndexToReadBlock function");
		}
		Bulk<byte> tailBuffer(_BlockSize);
		memset(tailBuffer, 0x00, _BlockSize);
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
		for (uint32 i = 0; i < direct_alloc_cnt; ++i) {
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
	uint32 indir_remain = _AddrPerBlock - indir_offset;
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
			uint need_cnt = div_ceil(child_cnt, _AddrPerBlock);

			/* The required size has been checked for leaf, so it is impossible to
					require exceed parent's capacity. */
			assert(need_cnt <= layer_max_size[layer]);

			layer_new_size[layer] = need_cnt;
			layer_alloc_size[layer] = need_cnt - layer_curr_size[layer];
		}
	}

	for (uint layer = 0; layer <= leaf_layer; ++layer) {
		vector<uint32> retBlocks;

		uint32 remain = BlockMan.AutoAllocBlock(layer_alloc_size[layer], retBlocks);

		Matrix[layer].insert(Matrix[layer].end(), retBlocks.begin(), retBlocks.end());

		if (layer == leaf_layer) {
			if (remain)
				throw CError(L"Out of block used");

			UpdateAddressTable(Matrix, BlockMan, retBlocks.size(), layer);

			return (RequireBlocks - layer_alloc_size[layer]);
		}
		else {
			assert(remain == 0);
		}
		UpdateAddressTable(Matrix, BlockMan, retBlocks.size(), layer);
	}
	return RequireBlocks;
}

/*
 * 	Update the address table data of the new adding blocks' parents.
 */
void CInode::UpdateAddressTable(CIndrMatrix& Matrix, CBlockManager& BlockMan,
															 uint32 NewCount, uint8 Layer)
{
	#define get_parent(x, y) (Matrix[x-1][y/_AddrPerBlock])
	#define to_local(x)   (x % _AddrPerBlock)

	if (Layer == 0) {
  	/* Update the root block data */
		uint32 root_block;

		if (Matrix.size() == 2)
			root_block = Inode.Blocks[EXT2_DIND_BLOCK];
		else
			root_block = Inode.Blocks[EXT2_TIND_BLOCK];

		Bulk<byte>* buffer = BlockMan.GetSingleBlockDataBuffer(root_block);

		if (buffer == NULL) {
			buffer = BlockMan.CreateSingleBlockDataBuffer(root_block);
		}
		memcpy(buffer->Data(), &Matrix[0][0], Matrix[0].size() * sizeof(uint32));
		return;
	}

	uint8 parent_layer = Layer - 1;

	vector<uint32>& parent_array = Matrix[parent_layer];
	vector<uint32>& child_array = Matrix[Layer];
	uint child_total = child_array.size();

	uint32 position = child_total - NewCount;
	uint32 local_pos = to_local(position);
	uint32 parent_block = get_parent(Layer, position);

	Bulk<byte>* buffer = BlockMan.GetSingleBlockDataBuffer(parent_block);

	if (buffer == NULL) {
    buffer = BlockMan.CreateSingleBlockDataBuffer(parent_block);
  }  
	uint32* ptr = (uint32*)buffer->Data();
	ptr += local_pos;

	while (position < child_total) {
		*ptr++ = child_array[position++];
    
		if (get_parent(Layer, position) != parent_block) {
			parent_block = get_parent(Layer, position);
			
			buffer = BlockMan.GetSingleBlockDataBuffer(parent_block);      

      if (buffer == NULL) {
				buffer = BlockMan.CreateSingleBlockDataBuffer(parent_block);
      }        
      ptr = (uint32*)buffer->Data();
		}
	}

}

uint32 CInode::GetLastRealBlock()
{
  int block_pos = CalcLeafCount() - 1;
  uint32 real;

  if (block_pos < 0 || (!IndexToRealBlock(block_pos, real))) {
    throw CError(L"No blocks allocated for the inqured inode");    
  }
  return real;
}

void CInode::make_layer_curr_sizes(uint32 Sizes[3], CIndrMatrix& Matrix)
{
	uint layer_cnt = Matrix.size();

	for (uint i = 0; i < 3; ++i) {
		if (i < layer_cnt)
			Sizes[i] = Matrix[i].size();
		else
			Sizes[i] = 0;
	}
}

void CInode::make_layer_max_sizes(uint32 Sizes[3])
{
	for (int i = 0; i < 3; ++i) {
		uint32 max = _AddrPerBlock;

		for (int f = 0; f < i; ++f) {
			max *= _AddrPerBlock;
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
    
		uint32 layer0 = offset / _AddrPerBlock;
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
		uint32 layer1 = offset / _AddrPerBlock;
		uint32 layer0 = layer1 / _AddrPerBlock;
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

/*
 * 	Validate the direct blocks table data.
 */
void CInode::ValidateDirectLink(CBlockManager& BlockMan)
{
  uint8 directCnt = Direct.size();

  if (memcmp((byte*)&Inode.Blocks[0], (byte*)&Direct[0], directCnt * sizeof(uint32)) != 0) {
    throw CError(L"Compare direct address block in the Inode table from the vector failed");  
  }
}


/*
 * 	Validate the indirect blocks table data.
 */
void CInode::ValidateIndirectLink(CBlockManager& BlockMan)
{
	uint block = Inode.Blocks[EXT2_IND_BLOCK];

	Bulk<byte>* buffer = BlockMan.GetSingleBlockDataBuffer(block);

	assert(buffer != NULL);

  uint indr_cnt = Indirect.size();

	if (memcmp(buffer->Data(), (uint8*)&Indirect[0], indr_cnt * sizeof(uint32)) != 0) {
   	throw CError(L"Compare indirect address block from the vector failed");
	}
}

/*
 * 	Validate the blocks for writing the address link of inode indirect trees.
 *	Compare the address table with the matrix.
 */
void CInode::ValidateMultilayerLink(CBlockManager& BlockMan, CIndrMatrix& Matrix)
{
	uint32 max_layer = Matrix.size();
	uint32 layer_size;
	uint32 root_block;

	if (max_layer == 2)
		root_block = Inode.Blocks[EXT2_DIND_BLOCK];
	else
    root_block = Inode.Blocks[EXT2_TIND_BLOCK];

	Bulk<byte>* buffer = BlockMan.GetSingleBlockDataBuffer(root_block);
	assert(buffer != NULL);

	uint count = Matrix[0].size();

	if (memcmp(buffer->Data(), (uint8*)&Matrix[0][0], count * sizeof(uint32)) != 0) {
		throw CError(L"Compare first layer address block from the matrix failed");
	}

	for (uint32 layer = 0; layer < (max_layer - 1); ++layer) {
		layer_size = Matrix[layer].size();
		uint32 next_layer_size = Matrix[layer+1].size();

		for (uint32 block = 0; block < layer_size; ++block) {
			uint32 addr_block = Matrix[layer][block];

			buffer = BlockMan.GetSingleBlockDataBuffer(addr_block);
			assert(buffer != NULL);

			uint child_offset = block * _AddrPerBlock;
			uint child_cnt = min_of(next_layer_size - child_offset, _AddrPerBlock);

			if (memcmp(buffer->Data(), (uint8*)&Matrix[layer+1][child_offset],
										child_cnt * sizeof(uint32)) != 0) {
				throw CError(L"Compare child layer address block from the matrix failed");
			}
		}
	}
}
/*
 *  The sector count recorded in the inode contains the data sectors and the
 *  indirect linking blocks.
 */
uint32 CInode::CalculateSectorCount(uint32 Size)
{ 
  uint32 total_blocks = div_ceil(Size, _BlockSize); 
  uint32 remain_blocks = total_blocks;
  uint32 sectors_per_block = _BlockSize / 512;

  uint32 dir_blocks = 0;
  uint32 ind_blocks = 0;
  uint32 dind_blocks[2] = {0, 0};
  uint32 tind_blocks[3] = {0, 0, 0};

  uint32 sectors = div_ceil(Size, 512);

  dir_blocks = min_of(EXT2_NDIR_BLOCKS, remain_blocks);
  remain_blocks -= dir_blocks;

  if (remain_blocks) {
    ind_blocks = min_of(_AddrPerBlock, remain_blocks);
    remain_blocks -= ind_blocks;

    sectors += sectors_per_block;

    if (remain_blocks) {
      uint32 max_dind_leaf = _AddrPerBlock * _AddrPerBlock;
      dind_blocks[1] = min_of(max_dind_leaf, remain_blocks);
      dind_blocks[0] = div_ceil(dind_blocks[1], _AddrPerBlock);
      remain_blocks -= dind_blocks[1];

      sectors += ((dind_blocks[0] + 1) * sectors_per_block);

      if (remain_blocks) {
        uint32 max_tind_leaf = _AddrPerBlock * _AddrPerBlock * _AddrPerBlock;
        tind_blocks[2] = min_of(max_tind_leaf, remain_blocks);
        tind_blocks[1] = div_ceil(tind_blocks[2], _AddrPerBlock);
        tind_blocks[0] = div_ceil(tind_blocks[1], _AddrPerBlock);

        sectors += (tind_blocks[1] * sectors_per_block);
        sectors += ((tind_blocks[0] + 1) * sectors_per_block);
      }
    }
  }  
  return sectors;
}


