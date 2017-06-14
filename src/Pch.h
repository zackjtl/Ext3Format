#ifndef PchH
#define PchH

#ifdef __BORLANDC__
//---------------------------------------------------------------------------
#include <cstdlib>
#include <typeinfo>
#include <bitset>
#include <functional>
#include <utility>
#include <ctime>
#include <cstddef>
#include <new>
#include <memory>
#include <limits>
#include <exception>
#include <cassert>
#include <cctype>
#include <cstring>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <algorithm>
#include <iterator>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <locale>
#include <vcl.h>
#include <windows.h>
#include <System.hpp>
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#ifdef _MSC_VER
//---------------------------------------------------------------------------
#include "Stdafx.h"
//---------------------------------------------------------------------------
#endif
#ifdef __GNUC__

#include <cstdlib>
#include <typeinfo>
#include <bitset>
#include <functional>
#include <utility>
#include <ctime>
#include <cstddef>
#include <new>
#include <memory>
#include <limits>
#include <exception>
#include <cassert>
#include <cctype>
#include <cstring>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <algorithm>
#include <iterator>
#include <cmath>
#include <iostream>
#include <fstream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <locale>
#include <stdio.h>
#include <wchar.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "BaseTypes.h"
//---------------------------------------------------------------------------
using namespace std;
using std::vector;
using std::ostream;
//---------------------------------------------------------------------------
#define FILE_BEGIN			SEEK_SET
#define FILE_CURRENT			SEEK_CUR
#define FILE_END			SEEK_END

#define FILE_SHARE_READ		0
#define FILE_SHARE_WRITE	0
#define FILE_SHARE_READ		0
#define FILE_SHARE_DELETE	0

#define GENERIC_READ		  O_RDONLY
#define GENERIC_WRITE	  	O_RDWR

#define OPEN_EXISTING     O_EXCL
#define CREATE_ALWAYS		  (O_TRUNC | O_CREAT)
#define OPEN_ALWAYS       (O_TRUNC | O_CREAT)

#define INVALID_HANDLE_VALUE -1

////#ifdef __GNUC__
//#define CUSTOM_UTILITY 1
#define Sleep(x){}
#define ZeroMemory(x,y)(memset(x, 0, y))
#define FALSE   false
#define TRUE    true
// #define BOOL    bool
////#endif

typedef int					HANDLE;		
typedef int					HWND;
typedef std::wstring		UnicodeString;

extern void FillMemory( void *ptr, size_t Length, uint8_t Fill );
extern void CloseHandle( HANDLE h );

extern std::string itoa(int val, char *buf, int radix);
extern std::wstring _itow(int val, wchar_t *buf, int radix);

#endif

#if defined(__GNUC__) || defined(__LIBSDK__)
typedef uint64_t			UHYPER;

extern std::wstring c2w(const char *pc);
extern std::string w2c(const wchar_t * pw);

extern bool trace_on;

extern std::string extract_fname(const char* Path);
extern std::wstring extract_fname(const wchar_t* Path);


#define tracer_msg(x){if (trace_on) {g_tracer << x << endl;}}

extern std::wstring g_DbFileName;
extern std::wofstream g_tracer;

bool FileExists(const wchar_t* pw);
bool FileExists(const char* pw);

#define __fastcall
#define __closure

#define USE_RESOURCES 1

#else
#define tracer_msg(x){}
#endif

#endif
