#ifndef InodeH
#define InodeH

#include "Types.h"
#include "BlockManager.h"
#include "Tables.h"
#include "Bulk.h"
#include <vector>

typedef std::vector<std::vector<uint32> >		CIndrMatrix;

class CInode
{
public:
	static CInode* Create(uint16 Mode, uint32 BlockSize);

	CInode(uint16 Mode, uint32 AddressPerBlock);
	~CInode();

	uint64 GetSize();
	uint64 GetPosition();

#ifdef _TEST_
public:	/* Public for testing. */
#else
protected:
#endif

	int WriteData(CBlockManager& BlockMan, byte* Buffer, int Length);

	uint32 alloc_blocks(CBlockManager& BlockMan, uint32 RequireBlock);
	uint32 alloc_dir_blocks(CBlockManager& BlockMan, uint32 RequireBlocks);
	uint32 alloc_indr_blocks(CBlockManager& BlockMan, uint32 RequireBlocks);
	uint32 alloc_dindr_blocks(CBlockManager& BlockMan, uint32 RequireBlocks);
	uint32 alloc_tindr_blocks(CBlockManager& BlockMan, uint32 RequireBlocks);

	uint32 GrowthTree(CIndrMatrix& Matrix, CBlockManager& BlockMan, uint32 RequireBlock);

	void make_layer_curr_sizes(uint32 Widths[3], CIndrMatrix& Matrix);
	void make_layer_max_sizes(uint32 Widths[3]);

	bool IndexToRealBlock(uint32 Index, uint32& Real);
	uint32 CalcLeafCount();
	void GetLeafBlocks(vector<uint32>&	Blocks);

	void UpdateAddressTable(CIndrMatrix& Matrix, CBlockManager& BlockMan,
													uint32 NewCount, uint8 Layer);

public:
	TInode		Inode;
	uint16 		Mode;
  uint32    BlockSize;
	uint32		AddrPerBlock;

  static const uint32 invalid_inode = 0xffffffff;  

#ifdef _TEST_
public:
#else
protected:
#endif

	vector<uint32>  Direct;
	vector<uint32>	Indirect;
	CIndrMatrix   	DIndirect;
	CIndrMatrix			TIndirect;

  const uint32        IndirectBlockThreshold;
  const uint32        DIndirectBlockThreshold;
	const uint32        TIndirectBlockThreshold;


  /* The position of current data pointer (byte unit) */
	uint64  _Position;
	uint64	_Size;
};

#endif
