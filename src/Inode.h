#ifndef InodeH
#define InodeH

#include "BaseTypes.h"
#include "BlockManager.h"
#include "e3fs_tables.h"
#include "Bulk.h"
#include <vector>

typedef std::vector<std::vector<uint32> >		CIndrMatrix;

class CInode
{
public:
	static CInode* Create(uint32 Type, uint16 BlockSize);

	CInode(uint32 Type, uint16 BlockSize);
	~CInode();

	uint64 GetSize();
	uint64 GetPosition();

	uint32 GetIndex();
	std::string GetName();

	void SetIndex(uint32 Index);
  void SetGroupID(uint16 GroupID);
	void SetName(const std::string& Name);
  void SetPermissions(uint16 Permissions);

  uint32 GetLastRealBlock();

  int SetData(CBlockManager& BlockMan, byte* Buffer, uint32 Length);
  virtual void UpdateInodeTable();

#ifdef _TEST_
public:	/* Public for testing. */
#else
protected:
#endif
	
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
	void GetLeafBlocks(std::vector<uint32>&	Blocks);

	void UpdateAddressTable(CIndrMatrix& Matrix, CBlockManager& BlockMan,
													uint32 NewCount, uint8 Layer);

  void ValidateDirectLink(CBlockManager& BlockMan);
	void ValidateIndirectLink(CBlockManager& BlockMan);
	void ValidateMultilayerLink(CBlockManager& BlockMan, CIndrMatrix& Matrix);

  uint32 CalculateSectorCount(uint32 Size);

public:
	TInode		Inode;
	uint32 		Type;  
  uint16    Permissions;
  
  uint16    _BlockSize;
	uint32		_AddrPerBlock;

  ////std::string    Permissions;

  static const uint32 invalid_inode = 0xffffffff;  

#ifdef _TEST_
public:
#else
protected:
#endif

  std::vector<uint32>   Direct;
  std::vector<uint32>	  Indirect;
	CIndrMatrix   	      DIndirect;
	CIndrMatrix			      TIndirect;

  const uint32        IndirectBlockThreshold;
  const uint32        DIndirectBlockThreshold;
	const uint32        TIndirectBlockThreshold;


  /* The position of current data pointer (byte unit) */
	uint64  _Position;
	uint64	_Size;

	uint32				_Index;
  uint16        _GroupID;
	std::string  	_Name;
};

#endif
