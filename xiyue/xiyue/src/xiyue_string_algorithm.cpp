#include "stdafx.h"
#include "xiyue_string_algorithm.h"

using namespace std;
using namespace xiyue;

//////////////////////////////////////////////////////////////////////////
// BM Ëã·¨
BoyerMooreStringMatcher::BoyerMooreStringMatcher(const wchar_t* pattern)
{
	new(this)BoyerMooreStringMatcher(pattern, wcslen(pattern));
}

BoyerMooreStringMatcher::BoyerMooreStringMatcher(const wchar_t* pattern, size_t patternLength)
{
	if (pattern == nullptr || patternLength == 0)
		return;

	m_pattern = pattern;
	m_patternLength = patternLength;

	m_bmGs = (size_t*)malloc(sizeof(size_t) * patternLength);
	m_suff = (size_t*)malloc(sizeof(size_t) * patternLength);

	prepareBmBc();
	prepareBmGs();
}

BoyerMooreStringMatcher::~BoyerMooreStringMatcher()
{
	free(m_suff);
	free(m_bmGs);
}

const wchar_t* BoyerMooreStringMatcher::searchIn(const wchar_t* str, size_t strLen, size_t startPos /*= 0*/)
{
	if (strLen < m_patternLength)
		return nullptr;

	if (m_patternLength == 0 || m_pattern == nullptr)
		return nullptr;

	while (startPos <= strLen - m_patternLength)
	{
		size_t location = m_patternLength - 1;
		const wchar_t* p = m_pattern + location;
		const wchar_t* pStr = str + startPos + location;
		while (p >= m_pattern)
		{
			if (*p != *pStr)
				break;

			--p;
			--pStr;
			--location;
		}

		if (p < m_pattern)
			return str + startPos;

		startPos += std::max(getBcStep(*pStr, location), m_bmGs[location]);
	}

	return nullptr;
}

void BoyerMooreStringMatcher::prepareBmBc()
{
	const wchar_t* pEnd = m_pattern + m_patternLength - 1;
	const wchar_t* p = m_pattern;
	size_t location = 0;
	while (p < pEnd)
	{
		m_bmBc[*p] = m_patternLength - 1 - location;
		++location;
		++p;
	}
}

void BoyerMooreStringMatcher::prepareBmGs()
{
	prepareSuff();

	for (size_t i = 0; i < m_patternLength; ++i)
	{
		m_bmGs[i] = m_patternLength - 1;
	}

	size_t j = 0;
	for (size_t i = 0; i != (size_t)(-1); --i)
	{
		if (m_suff[i] != i + 1)
			continue;

		for (; j < m_patternLength - 1 - i; ++j)
		{
			if (m_bmGs[i] == m_patternLength - 1)
				m_bmGs[i] = m_patternLength - i - 1;
		}
	}

	for (size_t i = 0; i < m_patternLength - 2; ++i)
	{
		m_bmGs[m_patternLength - 1 - m_suff[i]] = m_patternLength - i - 1;
	}
}

void BoyerMooreStringMatcher::prepareSuff()
{
	size_t f = 0, g;

	m_suff[m_patternLength - 1] = m_patternLength;
	g = m_patternLength - 1;
	const wchar_t* pStart = m_pattern;
	const wchar_t* p = m_pattern + m_patternLength - 2;
	size_t location = m_patternLength - 2;
	while (p >= pStart)
	{
		if (location > g && m_suff[location + m_patternLength - 1 - f] < location - g)
		{
			m_suff[location] = m_suff[location + m_patternLength - 1 - f];
		}
		else
		{
			if (location < g)
				g = location;
			f = location;
			while (g >= 0 && m_pattern[g] == m_pattern[g + m_patternLength - 1 - f])
				--g;
			m_suff[location] = f - g;
		}

		--location;
		--p;
	}
}

size_t BoyerMooreStringMatcher::getBcStep(wchar_t bc, size_t index)
{
	auto it = m_bmBc.find(bc);
	size_t step = it == m_bmBc.end() ? m_patternLength : it->second;
	size_t matchedLength = m_patternLength - 1 - index;
	return matchedLength >= step ? 1 : step - matchedLength;
}
