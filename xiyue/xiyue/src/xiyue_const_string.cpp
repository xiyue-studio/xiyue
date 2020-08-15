#include "stdafx.h"
#include <regex>
#include "xiyue_const_string.h"
#include "xiyue_encoding.h"
#include "xiyue_string_algorithm.h"

using namespace std;
using namespace xiyue;

static ConstString g_defaultDelimiter = L" \t\r\n\v\f"_cs;
static const wchar_t* g_emptyString = L"";

ConstStringPointer::ConstStringPointer(ConstString str)
	: m_string(str)
	, m_cursor(m_string.data())
{
	m_stringBase = m_cursor;
}

wchar_t ConstStringPointer::operator*()
{
	int offset = getOffset();
	if (offset < 0 || offset >= m_string.length())
		return 0;

	return *m_cursor;
}

ConstStringPointer ConstStringPointer::operator++(int)
{
	ConstStringPointer p(m_string);
	p.m_cursor = m_cursor;
	m_cursor++;

	return p;
}

ConstStringPointer& ConstStringPointer::operator++()
{
	m_cursor++;
	return *this;
}

ConstStringPointer ConstStringPointer::operator+(int offset)
{
	ConstStringPointer p(m_string);
	p.m_cursor = m_cursor + offset;
	return p;
}

ConstStringPointer ConstStringPointer::operator--(int)
{
	ConstStringPointer p(m_string);
	p.m_cursor = m_cursor;
	m_cursor--;

	return p;
}

ConstStringPointer& ConstStringPointer::operator--()
{
	m_cursor--;
	return *this;
}

ConstStringPointer ConstStringPointer::operator-(int offset)
{
	ConstStringPointer p(m_string);
	p.m_cursor = m_cursor - offset;
	return p;
}

ConstStringPointer& ConstStringPointer::operator+=(int offset)
{
	m_cursor += offset;
	return *this;
}

ConstStringPointer& ConstStringPointer::operator-=(int offset)
{
	m_cursor -= offset;
	return *this;
}

ConstStringPointer& ConstStringPointer::operator=(const ConstStringPointer& r)
{
	m_string = r.m_string;
	m_cursor = r.m_cursor;
	m_stringBase = m_string.data();
	return *this;
}

ConstString::ConstString(const wchar_t* str)
{
	new (this) ConstString(str, static_cast<int>(wcslen(str)));
}

ConstString::ConstString(const wchar_t* str, int len)
{
	m_start = 0;
	m_length = len;
	m_unmanagedStringData = nullptr;

	m_data = (ConstStringData*)malloc(sizeof(wchar_t) * (m_length + 1) + sizeof(ConstStringData));
	m_data->nRefCount = 1;
	m_data->nBufferSize = m_length + 1;
	memcpy(m_data->stringData(), str, sizeof(wchar_t) * m_length);
	m_data->stringData()[m_length] = 0;
}

ConstString::ConstString(const ConstString& str)
	: m_start(str.m_start)
	, m_length(str.m_length)
	, m_data(str.m_data)
	, m_unmanagedStringData(str.m_unmanagedStringData)
{
	if (m_data != nullptr)
		m_data->nRefCount++;
}

ConstString::ConstString(ConstString&& str)
	: m_start(str.m_start)
	, m_length(str.m_length)
	, m_data(str.m_data)
	, m_unmanagedStringData(str.m_unmanagedStringData)
{
	str.m_data = nullptr;
}

ConstString::ConstString()
	: m_start(0)
	, m_length(0)
	, m_data(nullptr)
	, m_unmanagedStringData(g_emptyString)
{
}

ConstString::ConstString(const std::wstring& str)
{
	new (this)ConstString(str.c_str(), str.size());
}

ConstString::~ConstString()
{
	if (m_data == nullptr)
		return;

	m_data->nRefCount--;
	if (m_data->nRefCount == 0)
		free(m_data);
}

ConstString ConstString::makeUnmanagedString(const wchar_t* str, int len)
{
	ConstString result;
	result.m_unmanagedStringData = str;
	result.m_length = len;
	return result;
}

ConstString ConstString::makeUnmanagedString(const wchar_t* str)
{
	return makeUnmanagedString(str, static_cast<int>(wcslen(str)));
}

wchar_t ConstString::operator[](int index) const
{
	index = normalizeIndex(index);
	if (index >= 0 && index < m_length)
		return data()[index];
	return 0;
}

int ConstString::normalizeIndex(int index) const
{
	return index < 0 ? m_length + index : index;
}

const wchar_t* ConstString::data() const
{
	return (m_data == nullptr ? m_unmanagedStringData : m_data->stringData()) + m_start;
}

ConstString& ConstString::operator=(const ConstString& str)
{
	m_start = str.m_start;
	m_length = str.m_length;
	m_unmanagedStringData = str.m_unmanagedStringData;
	m_data = str.m_data;
	if (m_data != nullptr)
		m_data->nRefCount++;

	return *this;
}

ConstString& ConstString::operator=(const wchar_t* str)
{
	long strSize = (long)wcslen(str);
	// 如果自己是单引用，并且内存空间足够，则直接覆盖原有内存
	if (m_data != nullptr && m_data->nRefCount == 1 && m_data->nBufferSize > strSize)
	{
		wcscpy_s(m_data->stringData(), strSize + 1, str);
		m_start = 0;
		m_length = (int)strSize;
	}
	else
	{
		return operator=(ConstString(str));
	}

	return *this;
}

bool ConstString::operator==(const ConstString& str) const
{
	if (m_length != str.m_length)
		return false;

	return wcsncmp(data(), str.data(), m_length) == 0;
}

bool ConstString::operator==(const wchar_t* str) const
{
	return wcsncmp(data(), str, m_length) == 0;
}

bool ConstString::operator!=(const ConstString& str) const
{
	return !operator==(str);
}

bool ConstString::operator!=(const wchar_t* str) const
{
	return !operator==(str);
}

bool ConstString::operator<(const ConstString& str) const
{
	int ret = wcsncmp(data(), str.data(), std::min(m_length, str.m_length));
	if (ret == 0)
		return m_length < str.m_length;
	return ret < 0;
}

bool ConstString::operator<(const wchar_t* str) const
{
	return operator<(ConstString(str));
}

ConstString ConstString::substr(int start, int size) const
{
	assert(start + size <= m_length);

	ConstString result = *this;
	result.m_start += start;
	result.m_length = size;

	return result;
}

ConstString ConstString::substr(int start) const
{
	return substr(start, length() - start);
}

ConstString ConstString::right(int size) const
{
	return substr(m_length - size, size);
}

ConstString ConstString::left(int size) const
{
	return substr(0, size);
}

ConstString ConstString::lTrim(const ConstString& trimChars /*= L""cs*/) const
{
	ConstString result = *this;
	const ConstString& delimiter = trimChars.isEmpty() ? g_defaultDelimiter : trimChars;
	wchar_t lastChar = 0;	// 如果连续两个字符一样，可以省去一次查找操作
	wchar_t ch = result[0];
	while (ch != 0 && (lastChar == ch || delimiter.containsChar(ch)))
	{
		result.m_start++;
		result.m_length--;
		lastChar = ch;
		ch = result[0];
	}

	return result;
}

ConstString ConstString::rTrim(const ConstString& trimChars /*= L""cs*/) const
{
	ConstString result = *this;
	const ConstString& delimiter = trimChars.isEmpty() ? g_defaultDelimiter : trimChars;
	wchar_t lastChar = 0;
	wchar_t ch = result[-1];
	while (ch != 0 && (lastChar == ch || delimiter.containsChar(ch)))
	{
		result.m_length--;
		lastChar = ch;
		ch = result[-1];
	}

	return result;
}

ConstString ConstString::trim(const ConstString& trimChars /*= L""cs*/) const
{
	return lTrim(trimChars).rTrim(trimChars);
}

int ConstString::find(const ConstString& str, int start /*= 0*/) const
{
	for (int i = start; i <= length() - str.length(); ++i)
	{
		bool found = true;
		for (int j = 0; j < str.length(); ++j)
		{
			if (str.data()[j] != data()[i])
			{
				found = false;
				break;
			}
		}

		if (found)
			return i;
	}

	return -1;
}

int ConstString::find(wchar_t ch, int start /*= 0*/) const
{
	const wchar_t* p = begin() + start;
	const wchar_t* pEnd = end();
	while (p != pEnd)
	{
		if (*p == ch)
			return (int)(p - begin());
	}

	return -1;
}

int ConstString::reverseFind(const ConstString& /*str*/, int /*start*/ /*= -1*/) const
{
	throw exception("Should be implement by KMP algorithm.");
}

int ConstString::reverseFind(wchar_t ch, int start /*= -1*/) const
{
	int index = normalizeIndex(start);
	const wchar_t* p = data() + index;
	const wchar_t* pEnd = data() - 1;
	while (p != pEnd)
	{
		if (*p == ch)
			return static_cast<int>(p - data());
		--p;
	}

	return -1;
}

bool ConstString::containsChar(wchar_t ch) const
{
	return !(std::find(data(), data() + length(), ch) == data() + length());
}

ConstString::operator const wchar_t*() const
{
	if (data() + length() == 0)
		return data();

	// 如果当前字符串不是 \0 结尾的，则重新创建缓冲区
	// 特别的，如果一个缓冲区是非共享的可修改缓冲区，则直接修改最后一个字符（创建缓冲区的时候都会多留一个字符）
	if (m_unmanagedStringData == nullptr && m_data->nRefCount == 1)
	{
		m_data->stringData()[m_start + m_length] = 0;
		return data();
	}

	// 其它情况，需要重建缓冲区
	ConstStringData* newData = (ConstStringData*)malloc(sizeof(wchar_t) * (length() + 1) + sizeof(ConstStringData));
	newData->nRefCount = 1;
	newData->nBufferSize = length() + 1;
	wcsncpy_s(newData->stringData(), length() + 1, data(), length());

	if (m_data != nullptr)
	{
		m_data->nRefCount--;
		if (m_data->nRefCount <= 0)
			free(m_data);
	}

	m_data = newData;
	m_start = 0;
	m_unmanagedStringData = nullptr;

	return data();
}

bool xiyue::operator==(const wchar_t* l, const ConstString& r)
{
	return r.operator==(l);
}

bool xiyue::operator<(const wchar_t* l, const ConstString& r)
{
	return wcsncmp(l, r.data(), r.m_length) < 0;

}

wstring xiyue::operator+(const wstring& l, ConstString r)
{
	wstring result = l;
	result.append(r.begin(), r.end());
	return result;
}

wstring& xiyue::operator+=(wstring& l, ConstString r)
{
	l.append(r.begin(), r.end());
	return l;
}

wstring xiyue::operator+(ConstString r, const std::wstring& l)
{
	wstring result(r.begin(), r.end());
	result.append(l);
	return result;
}

ConstString ConstString::makeByFormat(const wchar_t* format, ...)
{
	va_list args;
	va_start(args, format);

	int bufferCount = _vscwprintf(format, args);
	if (bufferCount < 0)
		return L""_cs;

	wchar_t* buffer = (wchar_t*)malloc(sizeof(wchar_t) * (bufferCount + 1));
	if (buffer == nullptr)
		return L""_cs;

	vswprintf_s(buffer, bufferCount + 1, format, args);
	ConstString result = buffer;
	free(buffer);
	va_end(args);

	return result;
}

ConstString ConstString::fromInteger(int val)
{
	return makeByFormat(L"%d", val);
}

const wchar_t* ConstString::end() const
{
	const wchar_t* p = begin();
	return p + length();
}

const wchar_t* ConstString::begin() const
{
	if (m_unmanagedStringData != nullptr)
		return m_unmanagedStringData + m_start;
	if (m_data != nullptr)
		return m_data->stringData() + m_start;
	else
		return nullptr;
}

double ConstString::toDouble() const
{
	return wtof(this->cstr());
}

int ConstString::toInt(int /*radius*/ /*= 10*/) const
{
	return (int)wtoi(this->cstr());
}

static int _hexCharToInt(wchar_t ch)
{
	if (isDigit(ch))
		return ch - '0';
	if (isLowerAlpha(ch))
		return ch - 'a' + 10;
	if (isUpperAlpha(ch))
		return ch - 'A' + 10;

	return 0;
}

ConstString ConstString::replaceEscapedChars() const
{
	if (isEmpty())
		return L""_cs;
	
	ConstString s;

	s.m_start = 0;
	s.m_length = 0;
	s.m_unmanagedStringData = nullptr;

	s.m_data = (ConstStringData*)malloc(sizeof(wchar_t) * (length() + 1) + sizeof(ConstStringData));
	s.m_data->nRefCount = 1;
	s.m_data->nBufferSize = length() + 1;
	
	// 遍历转义字符
	const wchar_t* p;
	wchar_t* pDst = s.m_data->stringData();
	if (m_unmanagedStringData != nullptr)
		p = m_unmanagedStringData + m_start;
	else
		p = m_data->stringData() + m_start;
	const wchar_t* pEnd = p + length();

	while (p < pEnd)
	{
		if (*p == 0)
			break;

		if (*p != '\\')
		{
			*pDst++ = *p++;
			continue;
		}

		// 处理转义字符
		p++;
		switch (*p)
		{
		case 'r':	// \r
			*pDst++ = '\r';
			break;
		case 'n':	// \n
			*pDst++ = '\n';
			break;
		case 't':	// \t
			*pDst++ = '\t';
			break;
		case 'v':	// \v
			*pDst++ = '\v';
			break;
		case 'f':	// \f
			*pDst++ = '\f';
			break;
		case 'x':	// \x00
			*pDst = 0;
			for (int i = 0; i < 2; ++i)
			{
				if (isHexDigit(*(p+1)))
				{
					*pDst = *pDst << 4;
					p++;
					*pDst += (wchar_t)_hexCharToInt(*p);
				}
			}
			break;
		case 'u':	// \u0000
			*pDst = 0;
			for (int i = 0; i < 4; ++i)
			{
				if (isHexDigit(*(p + 1)))
				{
					*pDst = *pDst << 4;
					p++;
					*pDst += (wchar_t)_hexCharToInt(*p);
				}
			}
			break;
		default:
			*pDst++ = *p;
			break;
		}

		p++;
	}

	*pDst = 0;
	s.m_length = pDst - s.m_data->stringData();

	return s;
}

ConstStringPointer ConstString::getPointer() const
{
	return ConstStringPointer(*this);
}

ConstString ConstString::operator+(ConstString r) const
{
	ConstString s;

	s.m_start = 0;
	s.m_length = this->length() + r.length();
	s.m_unmanagedStringData = nullptr;

	s.m_data = (ConstStringData*)malloc(sizeof(wchar_t) * (s.m_length + 1) + sizeof(ConstStringData));
	s.m_data->nRefCount = 1;
	s.m_data->nBufferSize = s.m_length + 1;
	memcpy(s.m_data->stringData(), data(), sizeof(wchar_t) * length());
	memcpy(s.m_data->stringData() + length(), r.data(), sizeof(wchar_t) * r.length());
	s.m_data->stringData()[s.m_length] = 0;

	return s;
}

ConstStringPointer ConstString::operator&()
{
	return getPointer();
}

ConstString ConstString::fromDouble(double val, const wchar_t* /*format*/ /*= nullptr*/)
{
	return makeByFormat(L"%f", val);
}

ConstString ConstString::duplicate() const
{
	return ConstString(data(), length());
}

ConstString ConstString::makeByReservedSize(size_t size)
{
	ConstString result;
	result.m_data = (ConstStringData*)malloc(sizeof(wchar_t) * (size + 1) + sizeof(ConstStringData));
	result.m_data->nBufferSize = size + 1;
	result.m_data->nRefCount = 1;
	result.m_unmanagedStringData = nullptr;
	return result;
}

ConstString ConstString::makeByRepeat(const ConstString& s, int repeatNum)
{
	ConstString sr = makeByReservedSize(s.length() * repeatNum);
	wchar_t* p = sr._getStringData()->stringData();
	for (int i = 0; i < repeatNum; ++i)
	{
		wcscpy_s(p, s.length() + 1, s.data());
		p += s.length();
	}

	return sr;
}

ConstString ConstString::makeByRepeat(const wchar_t* s, int repeatNum)
{
	return makeByRepeat(makeUnmanagedString(s, (int)wcslen(s)), repeatNum);
}

bool ConstString::canTransformToInt() const
{
	static wregex intRegex(L"\\d+");

	return regex_match(begin(), end(), intRegex);
}

bool ConstString::canTransformToDouble() const
{
	static wregex doubleRegex(LR"([-+]?(\d+\.?\d*|\.\d+)(e[-+]?\d+)?)", wregex::icase);

	return regex_match(begin(), end(), doubleRegex);
}

ConstString ConstString::join(const Array<ConstString>& elements, wchar_t seperator)
{
	return join(elements.begin(), elements.size(), seperator);
}

ConstString ConstString::join(const vector<ConstString>& elements, wchar_t seperator)
{
	return join(&*elements.begin(), elements.size(), seperator);
}

ConstString ConstString::join(const ConstString* elements, size_t elementCount, wchar_t seperator)
{
	if (elementCount == 0)
		return g_emptyString;

	size_t stringSize = 0;
	for (size_t i = 0; i < elementCount; ++i)
	{
		const ConstString* s = elements + i;
		stringSize += (size_t)s->length();
	}

	stringSize += elementCount - 1;

	ConstString result = ConstString::makeByReservedSize(stringSize);
	wchar_t* p = result._getStringData()->stringData();
	for (size_t i = 0; i < elementCount; ++i)
	{
		const ConstString* str = elements + i;
		wcsncpy_s(p, stringSize, str->begin(), str->length());
		p += str->length();

		if (i + 1 < elementCount)
			*p++ = seperator;
	}

	result._resetLength((int)stringSize);
	return result;
}

Array<ConstString> ConstString::split(const ConstString& seperators) const
{
	std::vector<ConstString> splitedParts;
	int startIndex = 0;
	int index = 0;
	for (wchar_t ch : *this)
	{
		if (seperators.find(ch) >= 0)
		{
			splitedParts.push_back(substr(startIndex, index - startIndex));
			startIndex = index + 1;
		}

		++index;
	}

	Array<ConstString> result(splitedParts.size());
	index = 0;
	for (auto& s : splitedParts)
	{
		result[index++] = s;
	}

	return result;
}

Array<ConstString> ConstString::split(wchar_t seperator) const
{
	std::vector<ConstString> splitedParts;
	int startIndex = 0;
	int index = 0;
	for (wchar_t ch : *this)
	{
		if (ch == seperator)
		{
			splitedParts.push_back(substr(startIndex, index - startIndex));
			startIndex = index + 1;
		}

		++index;
	}

	Array<ConstString> result(splitedParts.size());
	index = 0;
	for (auto& s : splitedParts)
	{
		result[index++] = s;
	}

	return result;
}

bool ConstString::equalsIgnoreCase(const ConstString& r) const
{
	if (length() != r.length())
		return false;

	for (int i = 0; i < length(); ++i)
	{
		if (tolower(data()[i]) != tolower(r.data()[i]))
			return false;
	}

	return true;
}

bool ConstString::equalsIgnoreCase(const wchar_t* r) const
{
	int len = (int)wcslen(r);
	if (length() != len)
		return false;

	for (int i = 0; i < len; ++i)
	{
		if (tolower(data()[i]) != tolower(r[i]))
			return false;
	}

	return true;
}

void ConstString::splitLines(std::vector<ConstString>& linesOut) const
{
	const wchar_t* p = begin();
	const wchar_t* pEnd = end();
	const wchar_t* pBegin = p;
	const wchar_t* pLineStart = p;

	for ( ;p < pEnd; ++p)
	{
		switch (*p)
		{
		case '\r':
			if (p + 1 < pEnd && *(p + 1) == '\n')
				break;
		case '\n':
			linesOut.push_back(substr(pLineStart - pBegin, p + 1 - pLineStart));
			pLineStart = p + 1;
			break;
		default:
			break;
		}
	}

	// last line
	if (pLineStart < pEnd)
		linesOut.push_back(substr(pLineStart - pBegin, pEnd - pLineStart));
}

ConstString ConstString::replaceAll(const ConstString& searchText, const ConstString& replacedText, int* replaceTimes /*= nullptr*/) const
{
	int rt = 0;
	BoyerMooreStringMatcher m(searchText.begin(), searchText.length());
	wstring result;
	size_t startPos = 0;
	const wchar_t* p;
	const wchar_t* pPrev = begin();
	while ((p = m.searchIn(begin(), length(), startPos)) != nullptr)
	{
		size_t index = p - begin();
		result.append(pPrev, p);
		result.append(replacedText.begin(), replacedText.end());
		++rt;
		pPrev = p + searchText.length();
		startPos = index + length();
	}

	if (replaceTimes != nullptr)
		*replaceTimes = rt;

	if (rt == 0)
		return *this;

	result.append(pPrev, end());
	return result;
}
