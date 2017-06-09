//---------------------------------------------------------------------------
#pragma hdrstop

#include "TestUUID.h"
#include "my_uuid.h"
#include "BaseError.h"
#include <iomanip>
#include <sstream>
using namespace std;
//---------------------------------------------------------------------------

void Test_uuid_v4_generator()
{
	byte uuid[16];

	for (int i = 0; i <32; ++i) {
		gen_uuid_v4(uuid, 16);

		if ((uuid[6] < 0x40) ||
				(uuid[6] & 0x40 == 0) ||
				(uuid[8] & 0x80 == 0)) {
			wostringstream  str;

			for (int i = 0; i < 16; ++i) {
				if ((i == 4) || (i == 6) || (i == 8) || (i == 10)) {
        	str << L"-";
				}
				str << hex << setw(2) << setfill(L'0') << uuid[i];
			}
			throw CError(L"Invalid uuid generated v4:\n " + str.str());
		}
	}
}
