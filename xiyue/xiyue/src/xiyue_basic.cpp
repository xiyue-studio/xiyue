#include "stdafx.h"
#include "xiyue_basic.h"
#include "xiyue_encoding.h"

#ifdef __linux__
#include <unistd.h>
#endif

using namespace std;
using namespace xiyue;

#ifdef WIN32
wchar_t* xiyue::itow(wchar_t* buffer, size_t length, int64_t number, int radix /*= 10*/)
{
	_i64tow_s(number, buffer, length, radix);
	return buffer;
}

int64_t xiyue::wtoi(const wchar_t* buffer)
{
	return _wtoi64(buffer);
}

wchar_t* xiyue::ftow(wchar_t* buffer, size_t length, double number)
{
	swprintf_s(buffer, length, L"%f", number);
	return buffer;
}

double xiyue::wtof(const wchar_t* buffer)
{
	return _wtof(buffer);
}

#endif

#ifdef __linux__

char* wtochar(const wchar_t *pSrc)
{
    int len = wcslen(pSrc) + 1;
    if (len <= 1)
		return nullptr;
	/*sizeof(wchar_t) = 4 */
	char* dest = (char*)malloc(len * sizeof(wchar_t));
	int ret = wcstombs(dest, pSrc, len * sizeof(wchar_t));
	
	if (ret <= 0)
	{
		free(dest);
		return NULL;
	}
	dest[ret] = '\0';
	return dest;
}

int _waccess(const wchar_t* path_s, int mode)
{
	char* path = wtochar(path_s);
	access(path, mode);
	free(path);
}

error_t _wfopen_s(FILE** fp, wchar_t const* filename_s, wchar_t const* mode_s)
{
	char* filename = wtochar(filename_s);
	char* mode = wtochar(mode_s);
	*fp = fopen(filename, mode);
	free(filename);
	free(mode);
	return errno;
}

#endif
