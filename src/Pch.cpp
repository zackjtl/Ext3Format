#include "Pch.h"
#if defined(__GNUC__)

#include <sys/stat.h>
#include <unistd.h>
#include <string>

string itoa(int val, char *buf, int radix)
{
	if( radix == 16 )
		sprintf( &buf[0], "%x", val );
	else
		sprintf( &buf[0], "%d", val );

	return string( buf );
}

wstring _itow(int val, wchar_t *buf, int radix)
{
	char tmpBuf[255] = {0x00};
	itoa( val, tmpBuf, radix );

	std::wstring wstr = c2w( tmpBuf );

	buf = (wchar_t*)wstr.c_str();
   // size_t destlen = mbstowcs(0,pc,0);

	//memcpy( buf, wstr, destlen );

	return wstr;
}



void FillMemory( void *ptr, size_t Length, BYTE Fill )
{
	memset( ptr, Fill, Length );
}

void CloseHandle( HANDLE h )
{
	if( h != INVALID_HANDLE_VALUE ) close( h );
}

std::string extract_fname(const char* Path)
{
  char* ssc;
  char* temp = new char[strlen(Path)+1];

  ::strncpy(temp, Path, strlen(Path)+1);

  char* psz = temp;

  int l = 0;
  ssc = strstr(psz, "/");
  do {
	l = strlen(ssc) + 1;
	psz = &psz[strlen(psz)-l+2];
	ssc = strstr(psz, "/");
  } while(ssc);

  string ret = psz;

  delete[] temp;

  return ret;
}


std::wstring extract_fname(const wchar_t* Path)
{
  wchar_t* ssc;
  wchar_t* temp = new wchar_t[wcslen(Path)+1];

  /*
   * Size need to +1 or there will be "garbled"
   */
  ::wcpncpy(temp, Path, wcslen(Path)+1);

  wchar_t* psz = temp;

  int l = 0;
  ssc = wcsstr(psz, L"/");
  do {
    l = wcslen(ssc) + 1;
    psz = &psz[wcslen(psz)-l+2];
    ssc = wcsstr(psz, L"/");
  } while(ssc);

  wstring ret = psz;

  delete[] temp;
  return ret;
}

#endif

#if defined(__GNUC__) || defined(__LIBSDK__)
std::wstring c2w(const char *pc)
{
	std::wstring val = L"";

	if(NULL == pc)
	{
		return val;
	}
	//size_t size_of_ch = strlen(pc)*sizeof(char);
	//size_t size_of_wc = get_wchar_size(pc);
	size_t size_of_wc;
	size_t destlen = mbstowcs(0,pc,0);
	if (destlen ==(size_t)(-1))
	{
		return val;
	}
	size_of_wc = destlen+1;
	wchar_t * pw  = new wchar_t[size_of_wc];
	mbstowcs(pw,pc,size_of_wc);
	val = pw;
	delete pw;
	return val;
}

/*
 wstring ËΩ¨Êç¢‰∏?string
 */
std::string w2c(const wchar_t * pw)
{
	std::string val = "";
	if(!pw)
	{
		return val;
	}
	size_t size= wcslen(pw)*sizeof(wchar_t);
	char *pc = NULL;
	if(!(pc = (char*)malloc(size)))
	{
		return val;
	}
	size_t destlen = wcstombs(pc,pw,size);
	/*ËΩ¨Êç¢‰∏ç‰∏∫Á©∫Êó∂ÔºåË??ûÂÄº‰∏∫-1?ÇÂ??ú‰∏∫Á©∫Ô?ËøîÂ???*/
	if (destlen ==(size_t)(0))
	{
		return val;
	}
	val = pc;
	delete pc;
	return val;
}

bool trace_on = false;
std::wstring g_DbFileName = L"";
std::wstring g_DiskName = L"";

std::wofstream g_tracer;
/*
 FileExists
 */
bool FileExists(const wchar_t* pw) 
{
  std::string str = w2c(pw);
  ifstream f(str.c_str());
  return f.good();    
}

/*
 FileExists
 */
bool FileExists(const char* pc) 
{
  ifstream f(pc);
  return f.good();    
}

#endif
