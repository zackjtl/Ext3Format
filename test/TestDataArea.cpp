#pragma hdrstop
#include "TestDataArea.h"
#include "DataArea.h"
#include "Types.h"
#include "BaseError.h"
#include <assert.h>

#include <vector>
using namespace std;

const int block_size = 4096;

void test_overlapped(void)
{
	CDataArea areas1[3] = {CDataArea(100, 60, block_size),   /* 100 ~ 159 */
												 CDataArea(159, 30, block_size),     /* 159 ~ 188 */
												 CDataArea(188, 60, block_size)};    /* 188 ~ 247 */

	CDataArea areas2[3] = {CDataArea(100, 60, block_size),   /* 100 ~ 159 */
												 CDataArea(160, 29, block_size),     /* 160 ~ 188 */
												 CDataArea(189, 60, block_size)};    /* 189 ~ 248 */

	if ((!overlapped_left(areas1[0], areas1[1])) ||
			(!overlapped_left(areas1[1], areas1[2]))) {
		throw CError(L"test overlapped_left macro");
	}

	if ((overlapped_left(areas2[0], areas2[1])) ||
			(overlapped_left(areas2[1], areas2[2]))) {
		throw CError(L"test not of overlapped_left macro");
	}

	if ((!overlapped(areas1[0], areas1[1])) ||
			(!overlapped(areas1[1], areas1[0])) ||
			(!overlapped(areas1[1], areas1[2])) ||
			(!overlapped(areas1[2], areas1[1]))) {
		throw CError(L"test overlapped macro");
	}

	if ((overlapped(areas2[0], areas2[1])) ||
			(overlapped(areas2[1], areas2[0])) ||
			(overlapped(areas2[1], areas2[2])) ||
			(overlapped(areas2[2], areas2[1]))) {
		throw CError(L"test not of overlapped_left macro");
	}
}

void test_can_merge(void)
{             
	CDataArea areas1[3] = {CDataArea(100, 60, block_size),   /* 100 ~ 159 */
												 CDataArea(160, 29, block_size),     /* 160 ~ 188 */
												 CDataArea(189, 60, block_size)};    /* 189 ~ 248 */

	CDataArea areas2[3] = {CDataArea(100, 59, block_size),   /* 100 ~ 158 */
												 CDataArea(160, 28, block_size),     /* 160 ~ 187 */
												 CDataArea(189, 60, block_size)};    /* 189 ~ 248 */
  
  if ((can_merge(areas2[0], areas2[1])) ||
      (can_merge(areas2[1], areas2[2])) ||
      (can_merge(areas1[1], areas1[0])) ||
      (can_merge(areas1[2], areas1[1]))){
		throw CError(L"test not of can_merge macro");
  }

  if ((!can_merge(areas1[0], areas1[1])) ||
      (!can_merge(areas1[1], areas1[2]))) {
    throw CError(L"test can_merge macro");
  }                                
}

#define init_test_data(x, y, ptr) { memset(x->GetData().Data(), y, x->GetData().Size()); \
																		memcpy(ptr, x->GetData().Data(), x->GetData().Size());\
																		ptr += x->GetData().Size();  }

#define get_areas_total_block(vec, res) {for (int i = 0; i < vec.size(); ++i) res += vec[i]->GetCount();}

/*
template< typename T, size_t N >
std::vector<T> make_vector( T (&data)[N] )
{
		return std::vector<T>(data, data+N);
} */

void test_merge(void)
{
	int total_cnt = 0;

	CDataArea::merge_assertion = true;

	std::vector<CDataArea*>	areas1;

	CDataArea*  x0 = new CDataArea(100, 60, block_size);
	CDataArea*  x1 = new CDataArea(160, 29, block_size);
	CDataArea*  x2 = new CDataArea(189, 60, block_size);

	areas1.push_back(x0);
	areas1.push_back(x1);
	areas1.push_back(x2);

	get_areas_total_block(areas1, total_cnt);

	Bulk<byte>  data(total_cnt * block_size);

	byte* ptr = data.Data();

	init_test_data(areas1[0], 0x55, ptr);
	init_test_data(areas1[1], 0xaa, ptr);
	init_test_data(areas1[2], 0x33, ptr);

	areas1[1]->MergeLeft(*areas1[0]);
	areas1[1]->MergeRight(*areas1[2]);

	if (areas1[1]->GetCount() != total_cnt) {
		throw CError(L"The left merged count is unexpected");
	}

	if (areas1[1]->GetData().Size() != data.Size()) {
		throw CError(L"The left merged data buffer size is unexpected");
	}

	if (memcmp(areas1[1]->GetData().Data(), data.Data(), data.Size()) != 0) {
		throw CError(L"The left merged data pattern is miscompared with the expected");
	}
	delete x0;
	delete x1;
	delete x2;
}

