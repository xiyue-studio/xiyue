#include "stdafx.h"
#include "xiyue_xy_re_instruction.h"
#include "xiyue_xy_re.h"
#include "xiyue_xy_re_parser.h"
#include "xiyue_xy_re_program_builder.h"
#include "xiyue_xy_re_vm.h"
#include "xiyue_encoding.h"

using namespace std;
using namespace xiyue;

//////////////////////////////////////////////////////////////////////////
// Sub Match
XyReSubMatch::XyReSubMatch(const ConstString& matchedString, uint32_t position)
	: m_matchedPosition(position)
	, m_matchedString(matchedString)
{
}

ConstString XyReSubMatch::getMatchedString() const
{
	return m_matchedString;
}

uint32_t XyReSubMatch::getMatchedPosition() const
{
	return m_matchedPosition;
}

uint32_t XyReSubMatch::getMatchedLength() const
{
	return m_matchedString.length();
}

bool XyReSubMatch::isEmpty() const
{
	return m_matchedString.isEmpty();
}

//////////////////////////////////////////////////////////////////////////
// Match
XyReMatch::XyReMatch()
{
	m_state = 0;
}

XyReMatch::XyReMatch(XyReMatch&& m)
{
	m_state = m.m_state;
	m_matchedPosition = m.m_matchedPosition;
	m_matchedLength = m.m_matchedLength;
	m_originalString = m.m_originalString;
	m_unnamedGroups.swap(m.m_unnamedGroups);
	m_namedGroups.swap(m.m_namedGroups);
}

uint32_t XyReMatch::getGroupCount() const
{
	return getNamedGroupCount() + getUnnamedGroupCount();
}

uint32_t XyReMatch::getUnnamedGroupCount() const
{
	return static_cast<uint32_t>(m_unnamedGroups.size() - 1);
}

uint32_t XyReMatch::getNamedGroupCount() const
{
	return static_cast<uint32_t>(m_namedGroups.size());
}

const XyReSubMatch& XyReMatch::getSubMatch(int index) const
{
	return m_unnamedGroups[index];
}

const XyReSubMatch& XyReMatch::getSubMatch(const ConstString& name) const
{
	auto it = m_namedGroups.find(name);
	if (it == m_namedGroups.end())
		throw range_error("Group name not found.");

	return it->second;
}

const XyReSubMatch& XyReMatch::getSubMatch(const wchar_t* name) const
{
	return getSubMatch(ConstString::makeUnmanagedString(name));
}

const XyReSubMatch& XyReMatch::getSubMatch(const wstring& name) const
{
	return getSubMatch(ConstString::makeUnmanagedString(name.c_str()));
}

bool XyReMatch::isReady() const
{
	return m_state != 0;
}

bool XyReMatch::isSuccess() const
{
	return m_state == 1;
}

ConstString XyReMatch::getMatchedString() const
{
	return isSuccess() ? m_originalString.substr(m_matchedPosition, m_matchedLength) : L""_cs;
}

uint32_t XyReMatch::getMatchedPosition() const
{
	return m_matchedPosition;
}

uint32_t XyReMatch::getMatchedLength() const
{
	return m_matchedLength;
}

ConstString XyReMatch::getPrefixString() const
{
	return isSuccess() ? m_originalString.substr(0, m_matchedPosition) : L""_cs;
}

ConstString XyReMatch::getSuffixString() const
{
	return isSuccess() ? m_originalString.substr(m_matchedPosition + m_matchedLength) : L""_cs;
}

ConstString XyReMatch::format(const wchar_t* formatStr) const
{
	return format(ConstString::makeUnmanagedString(formatStr));
}
enum ReplaceCharMode
{
	NORMAL_CASE,
	UPPER_CASE,
	LOWER_CASE
};

static force_inline void replaceByCaseMode(wstring& target, const ConstString& str, ReplaceCharMode mode)
{
	const wchar_t* begin = str.begin();
	const wchar_t* end = str.end();

	if (mode == NORMAL_CASE)
	{
		target.append(begin, end);
	}
	else if (mode == UPPER_CASE)
	{
		while (begin < end)
		{
			target.push_back(towupper(*begin));
			++begin;
		}
	}
	else
	{
		assert(mode == LOWER_CASE);
		while (begin < end)
		{
			target.push_back(towlower(*begin));
			++begin;
		}
	}
}

ConstString XyReMatch::format(const ConstString& formatStr) const
{
	if (formatStr.isEmpty())
		return L""_cs;

	const wchar_t* fsBegin = formatStr.begin();
	const wchar_t* fsEnd = formatStr.end();
	ReplaceCharMode caseMode = NORMAL_CASE;
	wstring result;
	result.reserve(formatStr.length());

	for (const wchar_t* p = fsBegin; p < fsEnd; ++p)
	{
		if (*p != '$')
		{
			result.push_back(*p);
			continue;
		}

		++p;
		if (p == fsEnd)
			break;
		if (isDigit(*p))
		{
			size_t num = *p - '0';
			const wchar_t* pNumStart = p;
			++p;
			while (p < fsEnd && isDigit(*p))
			{
				num = num * 10 + (*p - '0');
				++p;
			}
			--p;
			if (num > m_unnamedGroups.size())
			{
				// �������Ǹ���Ч����ţ���ֱ��ԭ�����
				result.append(pNumStart, p + 1);
			}
			else
			{
				// �滻Ϊ�����������
				ConstString subMatch = getSubMatch(num).getMatchedString();
				replaceByCaseMode(result, subMatch, caseMode);
			}
		}
		else if (*p == '{')
		{
			const wchar_t* nameStart = p;
			while (p < fsEnd && *p != '}')
				++p;
			if (*p != '}')
			{
				// ������������ã������ {
				p = nameStart;
				result.push_back('{');
				continue;
			}
			ConstString name(nameStart + 1, p - nameStart - 1);
			// ����Ǹ����֣��������ֲ����鴦��
			const wchar_t* np = name.begin();
			bool isAllNum = !name.isEmpty();
			size_t num = 0;
			while (np < name.end())
			{
				if (!isDigit(*np))
				{
					isAllNum = false;
					break;
				}
				num = num * 10 + (*np - '0');
				++np;
			}
			if (isAllNum)
			{
				// �����������滻
				if (num > m_unnamedGroups.size())
				{
					// �������Ǹ���Ч����ţ���ֱ��ԭ�����
					result.append(nameStart, p + 1);
				}
				else
				{
					// �滻Ϊ�����������
					ConstString subMatch = getSubMatch(num).getMatchedString();
					replaceByCaseMode(result, subMatch, caseMode);
				}
			}
			else
			{
				// �����������滻
				auto it = m_namedGroups.find(name);
				if (it == m_namedGroups.end())
				{
					// ��Ч���ƣ��滻Ϊԭ����
					result.append(nameStart, p + 1);
				}
				else
				{
					// �滻Ϊ����������
					ConstString subMatch = getSubMatch(name).getMatchedString();
					replaceByCaseMode(result, subMatch, caseMode);
				}
			}
		}
		else
		{
			switch (*p)
			{
			case '$':
				result.push_back('$');
				break;
			case '&':
				replaceByCaseMode(result, getMatchedString(), caseMode);
				break;
			case '`':
				replaceByCaseMode(result, getPrefixString(), caseMode);
				break;
			case '\'':
				replaceByCaseMode(result, getSuffixString(), caseMode);
				break;
			case '+':
				if (!m_unnamedGroups.empty())
					replaceByCaseMode(result, getSubMatch(m_unnamedGroups.size() - 1).getMatchedString(), caseMode);
				break;
			case '_':
				replaceByCaseMode(result, m_originalString, caseMode);
				break;
			case 'U':
				caseMode = UPPER_CASE;
				break;
			case 'E':
				caseMode = NORMAL_CASE;
				break;
			case 'L':
				caseMode = LOWER_CASE;
				break;
			default:
				result.push_back(*p);
				break;
			}
		}
	}

	return ConstString(result);
}

//////////////////////////////////////////////////////////////////////////
// XyRe
XyRe::XyRe(const wchar_t* regStrBegin, const wchar_t* regStrEnd, const wchar_t* flags /*= nullptr*/)
{
	new(this)XyRe(ConstString(regStrBegin, regStrEnd - regStrBegin), flags);
}

XyRe::XyRe(const wchar_t* regStr, const wchar_t* flags /*= nullptr*/)
{
	new(this)XyRe(ConstString(regStr, wcslen(regStr)), flags);
}

XyRe::XyRe(const ConstString& regStr, const wchar_t* flags /*= nullptr*/)
	: m_regStr(regStr)
{
	m_regData = nullptr;
	m_isIgnoreCase = false;
	m_isGlobalSearchMode = false;
	m_isDotMatchNewLine = false;
	m_isMultiLineMode = false;
	m_isLooseMode = false;
	m_isUnicodeMode = false;
	m_isNoCaptureMode = false;
	m_isRegDataManaged = true;

	if (flags == nullptr)
		return;

	for (wchar_t f = *flags; f != 0; f = *++flags)
	{
		switch (f)
		{
		case XyReMatch_ignoreCase:
			m_isIgnoreCase = true;
			break;
		case XyReMatch_globalSearch:
			m_isGlobalSearchMode = true;
			break;
		case XyReMatch_dotMatchNewLine:
			m_isDotMatchNewLine = true;
			break;
		case XyReMatch_multiLine:
			m_isMultiLineMode = true;
			break;
		case XyReMatch_looseMode:
			m_isLooseMode = true;
			break;
		case XyReMatch_unicode:
			m_isUnicodeMode = true;
			break;
		case XyReMatch_noCaptureGroup:
			m_isNoCaptureMode = true;
			break;
		}
	}
}

XyRe::~XyRe()
{
	if (m_isRegDataManaged)
		free(m_regData);
}

void XyRe::compile()
{
	if (m_regData != nullptr)
		return;

	XyReParser parser;
	auto ast = parser.parse(m_regStr);
	XyReProgramBuilder builder;
	m_regData = builder.build(ast.get());
	m_isRegDataManaged = true;
}

bool XyRe::match(const ConstString& str, XyReMatch* matchOut)
{
	compile();

	XyReVM vm(reinterpret_cast<const XyReProgram*>(m_regData));
	return vm.match(str, matchOut);
}

bool XyRe::match(const wchar_t* str, XyReMatch* matchOut)
{
	return match(ConstString(str), matchOut);
}

bool XyRe::match(const wchar_t* begin, const wchar_t* end, XyReMatch* matchOut)
{
	return match(ConstString(begin, end - begin), matchOut);
}

bool XyRe::match(const wstring& str, XyReMatch* matchOut)
{
	return match(ConstString(str), matchOut);
}

bool XyRe::search(const ConstString& str, XyReMatch* matchOut, int startIndex /*= 0*/)
{
	compile();

	XyReVM vm(reinterpret_cast<const XyReProgram*>(m_regData));
	return vm.searchOne(str, startIndex, matchOut);
}

bool XyRe::search(const wchar_t* str, XyReMatch* matchOut, int startIndex /*= 0*/)
{
	return search(ConstString(str), matchOut, startIndex);
}

bool XyRe::search(const wchar_t* begin, const wchar_t* end, XyReMatch* matchOut, int startIndex /*= 0*/)
{
	return search(ConstString(begin, end - begin), matchOut, startIndex);
}

bool XyRe::search(const wstring& str, XyReMatch* matchOut, int startIndex /*= 0*/)
{
	return search(ConstString(str), matchOut, startIndex);
}

vector<XyReMatch> XyRe::search(const ConstString& str)
{
	compile();

	XyReVM vm(reinterpret_cast<const XyReProgram*>(m_regData));
	vector<XyReMatch> results;
	vm.searchGlobal(str, m_isGlobalSearchMode, results);
	return move(results);
}

vector<XyReMatch> XyRe::search(const wchar_t* str)
{
	return move(search(ConstString(str)));
}

vector<XyReMatch> XyRe::search(const wchar_t* begin, const wchar_t* end)
{
	return move(search(ConstString(begin, end - begin)));
}

vector<XyReMatch> XyRe::search(const wstring& str)
{
	return move(search(ConstString(str)));
}

bool XyRe::testMatch(const ConstString& str)
{
	compile();

	XyReVM vm(reinterpret_cast<const XyReProgram*>(m_regData));
	return vm.test(str, 0, true);
}

bool XyRe::testMatch(const wstring& str)
{
	return testMatch(ConstString(str));
}

bool XyRe::testMatch(const wchar_t* str)
{
	return testMatch(ConstString(str));
}

bool XyRe::testMatch(const wchar_t* begin, const wchar_t* end)
{
	return testMatch(ConstString(begin, end - begin));
}

uint32_t XyRe::testSearch(const ConstString& str, int startIndex /*= 0*/)
{
	compile();

	XyReVM vm(reinterpret_cast<const XyReProgram*>(m_regData));
	return vm.test(str, startIndex, false);
}

uint32_t XyRe::testSearch(const wstring& str, int startIndex /*= 0*/)
{
	return testSearch(ConstString(str), startIndex);
}

uint32_t XyRe::testSearch(const wchar_t* str, int startIndex /*= 0*/)
{
	return testSearch(ConstString(str), startIndex);
}

uint32_t XyRe::testSearch(const wchar_t* begin, const wchar_t* end, int startIndex /*= 0*/)
{
	return testSearch(ConstString(begin, end - begin), startIndex);
}

ConstString XyRe::replace(const ConstString& srcStr, const ConstString& replacePattern)
{
	// TODO �� replacePattern �������滻ָ��Ա����ظ� format ����
	vector<XyReMatch> matches = search(srcStr);
	wstring result;
	const wchar_t* str = srcStr.begin();
	for (XyReMatch& m : matches)
	{
		result.append(str, srcStr.begin() + m.getMatchedPosition());
		ConstString mf = m.format(replacePattern);
		result.append(mf.begin(), mf.end());
		str = srcStr.begin() + m.getMatchedPosition() + m.getMatchedLength();
	}
	result.append(str, srcStr.end());

	return ConstString(result);
}

uint32_t* XyRe::buildProgram(const wchar_t* re)
{
	XyReParser parser;
	auto ast = parser.parse(re);
	XyReProgramBuilder builder;
	uint32_t* result = builder.build(ast.get());
	return result;
}
