#include "stdafx.h"
#include "xiyue_encoding.h"

using namespace xiyue;
using namespace std;

using EncodingTransformFunction = size_t(*)(const void* src, void* dst, size_t dstSize);

static size_t _utf16LEToUtf8(const void* src, void* dst, size_t dstSize);
static size_t _utf16LEToGb2312(const void* src, void* dst, size_t dstSize);
static size_t _utf8ToUtf16LE(const void* src, void* dst, size_t dstSize);
static size_t _utf8ToGb2312(const void* src, void* dst, size_t dstSize);
static size_t _gb2312ToUtf8(const void* src, void* dst, size_t dstSize);
static size_t _gb2312ToUtf16LE(const void* src, void* dst, size_t dstSize);

static size_t _equalMultiBytesEncoding(const void* src, void* dst, size_t dstSize);
static size_t _equalWideCharEncoding(const void* src, void* dst, size_t dstSize);

static inline int _makeEncodingPair(StringEncoding src, StringEncoding dst)
{
	return (((int)src) << 16) + ((int)dst);
}

static unordered_map<int, EncodingTransformFunction> g_encodingTransformFunctions;

static void _initEncodingTransformFunctionMap()
{
#define addFunctionMap(src, dst, func) g_encodingTransformFunctions.insert(make_pair(_makeEncodingPair(src, dst), func))
	addFunctionMap(StringEncoding_utf16LE, StringEncoding_utf8, _utf16LEToUtf8);
	addFunctionMap(StringEncoding_utf16LE, StringEncoding_gb2312, _utf16LEToGb2312);
	addFunctionMap(StringEncoding_utf8, StringEncoding_utf16LE, _utf8ToUtf16LE);
	addFunctionMap(StringEncoding_utf8, StringEncoding_gb2312, _utf8ToGb2312);
	addFunctionMap(StringEncoding_gb2312, StringEncoding_utf8, _gb2312ToUtf8);
	addFunctionMap(StringEncoding_gb2312, StringEncoding_utf16LE, _gb2312ToUtf16LE);

	addFunctionMap(StringEncoding_utf8, StringEncoding_utf8, _equalMultiBytesEncoding);
	addFunctionMap(StringEncoding_gb2312, StringEncoding_gb2312, _equalMultiBytesEncoding);
	addFunctionMap(StringEncoding_utf16LE, StringEncoding_utf16LE, _equalWideCharEncoding);
#undef addFunctionMap
}

size_t xiyue::xiyue_transformStringEncoding(const void* src, void* dst, size_t dstSize,
	StringEncoding srcEncoding, StringEncoding dstEncoding)
{
	assert(srcEncoding != StringEncoding_unspecifiedEncoding && dstEncoding != StringEncoding_unspecifiedEncoding);

	// 跳过源 BOM
	switch (srcEncoding)
	{
	case StringEncoding_utf8WithBOM:
		src = static_cast<const char*>(src) + 3;
		srcEncoding = StringEncoding_utf8;
		break;
	case StringEncoding_utf16LEWithBOM:
		src = static_cast<const char*>(src) + 2;
		srcEncoding = StringEncoding_utf16LE;
		break;
	case StringEncoding_utf16BEWithBOM:
		src = static_cast<const char*>(src) + 2;
		srcEncoding = StringEncoding_utf16BE;
		break;
	default:
		break;
	}

	// 获取目标的 BOM
	int sizeOfBOM = 0;
	const char* strBOM = nullptr;
	switch (dstEncoding)
	{
	case StringEncoding_utf8WithBOM:
		sizeOfBOM = 3;
		strBOM = UTF8_BOM;
		dstEncoding = StringEncoding_utf8;
		break;
	case StringEncoding_utf16LEWithBOM:
		sizeOfBOM = 2;
		strBOM = UTF16_LE_BOM;
		dstEncoding = StringEncoding_utf16LE;
		break;
	case StringEncoding_utf16BEWithBOM:
		sizeOfBOM = 2;
		strBOM = UTF16_BE_BOM;
		dstEncoding = StringEncoding_utf16BE;
		break;
	default:
		break;
	}

	// 获取转换函数
	EncodingTransformFunction func;
	if (g_encodingTransformFunctions.size() == 0)
		_initEncodingTransformFunctionMap();

	auto it = g_encodingTransformFunctions.find(_makeEncodingPair(srcEncoding, dstEncoding));
	if (it == g_encodingTransformFunctions.end())
	{
		// LOG ERROR
		return 0;
	}
	func = it->second;

	// 转换字符串
	if (dst && sizeOfBOM > 0)
	{
		memcpy(dst, strBOM, sizeOfBOM);
		dst = static_cast<char*>(dst) + sizeOfBOM;
	}

	return func(src, dst, dstSize) + sizeOfBOM;
}

void* xiyue::xiyue_transformStringEncoding(const void* src, StringEncoding srcEncoding, StringEncoding dstEncoding)
{
	size_t length = xiyue_transformStringEncoding(src, nullptr, 0, srcEncoding, dstEncoding);
	if (length == 0)
		return nullptr;

	void* result = nullptr;

	switch (dstEncoding)
	{
	case StringEncoding_utf8:
	case StringEncoding_gb2312:
	case StringEncoding_utf8WithBOM:
		result = malloc(length);
		break;
	default:
		result = malloc(length);
		break;
	}

	xiyue_transformStringEncoding(src, result, length, srcEncoding, dstEncoding);
	return result;
}

StringEncoding xiyue::xiyue_getEncodingFromBOM(const void* bytes, size_t size)
{
	if (size < 2)
		return StringEncoding_unspecifiedEncoding;

	if (size > 2 && memcmp(bytes, UTF8_BOM, 3) == 0)
		return StringEncoding_utf8WithBOM;

	if (memcmp(bytes, UTF16_LE_BOM, 2) == 0)
		return StringEncoding_utf16LEWithBOM;

	if (memcmp(bytes, UTF16_BE_BOM, 2) == 0)
		return StringEncoding_utf16BEWithBOM;

	return StringEncoding_unspecifiedEncoding;
}

#ifdef _WIN32
size_t _utf16LEToUtf8(const void* src, void* dst, size_t dstSize)
{
	return WideCharToMultiByte(CP_UTF8, 0, (LPCWCH)src, -1,
		(LPSTR)dst, (int)dstSize, nullptr, nullptr);
}

size_t _utf16LEToGb2312(const void* src, void* dst, size_t dstSize)
{
	return WideCharToMultiByte(CP_ACP, 0, (LPCWCH)src, -1,
		(LPSTR)dst, (int)dstSize, nullptr, nullptr);
}

size_t _utf8ToUtf16LE(const void* src, void* dst, size_t dstSize)
{
	return MultiByteToWideChar(CP_UTF8, 0, (LPCCH)src, -1,
		(LPWSTR)dst, (int)(dstSize / sizeof(wchar_t))) * sizeof(wchar_t);
}

size_t _gb2312ToUtf16LE(const void* src, void* dst, size_t dstSize)
{
	return MultiByteToWideChar(CP_ACP, 0, (LPCCH)src, -1,
		(LPWSTR)dst, (int)(dstSize / sizeof(wchar_t))) * sizeof(wchar_t);
}

size_t _utf8ToGb2312(const void* src, void* dst, size_t dstSize)
{
	LPSTR dstPtr = static_cast<LPSTR>(dst);
	LPWSTR tpDst = nullptr;
	int tpDstSize = 0;
	
	tpDstSize = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)src, -1, tpDst, tpDstSize);
	tpDst = (LPWSTR)malloc(sizeof(wchar_t) * tpDstSize);
	MultiByteToWideChar(CP_UTF8, 0, (LPCCH)src, -1, tpDst, tpDstSize);

	dstSize = (size_t)WideCharToMultiByte(CP_ACP, 0, tpDst, -1,
		dstPtr, (int)dstSize, nullptr, nullptr);

	free(tpDst);
	return dstSize;
}

size_t _gb2312ToUtf8(const void* src, void* dst, size_t dstSize)
{
	LPSTR dstPtr = static_cast<LPSTR>(dst);
	LPWSTR tpDst = nullptr;
	int tpDstSize = 0;

	tpDstSize = MultiByteToWideChar(CP_ACP, 0, (LPCCH)src, -1, tpDst, tpDstSize);
	tpDst = (LPWSTR)malloc(sizeof(wchar_t) * tpDstSize);
	MultiByteToWideChar(CP_UTF8, 0, (LPCCH)src, -1, tpDst, tpDstSize);

	dstSize = (size_t)WideCharToMultiByte(CP_UTF8, 0, tpDst, -1,
		dstPtr, (int)dstSize, nullptr, nullptr);

	free(tpDst);
	return dstSize;
}

size_t _equalMultiBytesEncoding(const void* src, void* dst, size_t)
{
	LPCCH srcPtr = static_cast<LPCCH>(src);
	LPCH dstPtr = static_cast<LPCH>(dst);

	size_t strLength = strlen(srcPtr);
	if (dstPtr != nullptr)
		memcpy(dstPtr, srcPtr, strLength + 1);

	return strLength + 1;
}

static size_t _equalWideCharEncoding(const void* src, void* dst, size_t)
{
	LPCWCH srcPtr = static_cast<LPCWCH>(src);
	LPWCH dstPtr = static_cast<LPWCH>(dst);

	size_t strLength = wcslen(srcPtr) * sizeof(wchar_t);
	if (dstPtr != nullptr)
		memcpy(dstPtr, srcPtr, strLength + sizeof(wchar_t));

	return strLength + sizeof(wchar_t);
}
#endif