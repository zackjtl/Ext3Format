//---------------------------------------------------------------------------
#pragma hdrstop
#include "ResizeInode.h"
#include "BaseError.h"
#include "BlockGroup.h"
#include "TypeConv.h"
#include "GlobalDef.h"
#include <time.h>
#include <cstring>
//---------------------------------------------------------------------------

CResizeInode::CResizeInode(uint16 BlockSize)
  : CInode(LINUX_S_IFREG, BlockSize),
    _written(false)
{
  Permissions = 600;
}

/*
 *  Specialize the write data method for Resize Inode. 
 */
void CResizeInode::WriteData(CBlockManager& BlockMan, TSuperBlock& Super, CExt2Params& Params)
{
  if (_written) 
    throw CError(L"The resize inode can written only once.");
  
  uint32 rootBlock;
  
  if (!BlockMan.AutoAllocSingleBlock(rootBlock))
    throw CError(L"Out of block to allocate resize inode root block"); 

  uint32 gdtBlockEnd = Params.RsvdGdtBlockOffset + Super.RsvdGdtBlocks;
  
  Inode.Blocks[EXT2_DIND_BLOCK] = rootBlock;

  DIndirect.resize(2);
  DIndirect[0].resize(gdtBlockEnd);

  #define occupied_wo_write(x) (BlockMan.HasBlockOccupied(x) && (!BlockMan.HasBlockWritten(x)))

  for (uint32 i = 0; i < gdtBlockEnd; ++i) {
		if (i < Params.RsvdGdtBlockOffset)
      DIndirect[0][i] = 0;
    else {
      /* The gdt address table blocks should be occupied earlier but not written */
			if (!occupied_wo_write(i))
				throw CError(L"Gdt address table block not occupied or has been written");

      _middle_block_cnt = 0;
			DIndirect[0][i-1] = i;
      Bulk<byte> buffer(_BlockSize);
      memset(buffer.Data(), 0, buffer.Size());
      uint32* ptr = (uint32*)buffer.Data();

      for (uint32 gp = 1; gp < Params.GroupCount; ++gp) {
        if (CBlockGroup::bg_has_super(Super, gp)) {
					uint32 rsvdGdtBlock = gp * Params.BlocksPerGroup +
																				Params.RsvdGdtBlockOffset + (i - Params.RsvdGdtBlockOffset);
          /* The reserved gdt blocks should be occupied earlier but not written */
          if (!occupied_wo_write(rsvdGdtBlock))
						throw CError(L"The reserved gdt block not occupied or has been written");

          Bulk<byte>* tempBuff = BlockMan.CreateSingleBlockDataBuffer(rsvdGdtBlock);
          memset(tempBuff->Data(), 0xFF, tempBuff->Size());          

          *ptr++ = rsvdGdtBlock;
          ++_middle_block_cnt;
				}
      }
			BlockMan.SetSingleBlockData(i, buffer.Data(), buffer.Size());
    }    
  }
  
  Bulk<byte>* rootData = BlockMan.CreateSingleBlockDataBuffer(rootBlock);

  memset(rootData->Data(), 0x00, rootData->Size());
  memcpy(rootData->Data(), (byte*)&DIndirect[0][0], DIndirect[0].size() * sizeof(uint32));
  
  _written = true;
}

void CResizeInode::UpdateInodeTable()
{
  _Size = DIndirect[0].size() * _middle_block_cnt * _BlockSize;

  Inode.Mode = OctToDec(Type | Permissions);
  Inode.Uid = 0;
  Inode.SizeInBytesLo = 4243456;
  Inode.SizeInBytesHi = 1;
  Inode.AccessTime = GetPosixTime();
  Inode.InodeChangeTime = Inode.AccessTime;
  Inode.ModificationTime = Inode.AccessTime;
  Inode.DeleteTime = 0;
  Inode.GroupId = _GroupID;
  Inode.SectorCount = div_ceil(_Size, 512);
  Inode.FileFlags = 0;
  Inode.HardLinkCnt = 1;  /* For regular file, this is always 1 */
  Inode.OS_Dep1 = 0;
  Inode.FileVersion = 0;
  Inode.FragAddress = 0;
  Inode.FragNumber = 0;
  Inode.FragSize = 0;
  Inode.Padding = 0;
  memset((byte*)&Inode.Rsvd[0], 0, sizeof(Inode.Rsvd));  
}

