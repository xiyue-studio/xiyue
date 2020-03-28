#include "stdafx.h"
#include "xiyue_basic.h"

using namespace std;
using namespace xiyue;

#ifdef _WIN32
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
