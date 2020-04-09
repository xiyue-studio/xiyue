#include "stdafx.h"
#include "xiyue_string_format.h"
#include "xiyue_encoding.h"

using namespace std;
using namespace xiyue;

bool xiyue::xiyue_formatNumber(std::wstring& output, const wchar_t* formatStr, double value)
{
	/*
		转换 format，计算整数部分最少留几位，小数部分最少留几位，最多留几位。

		整数部分第一个 0 往后一直到小数点，就是整数部分最少的位数。
		小数点往后一直到尾就是小数部分最多留的位数。
		小数点往后到最后一个 0 的长度就是小数部分最少留几位。
	*/
	const wchar_t* p = formatStr;
	const wchar_t* p0 = nullptr;
	while (*p)
	{
		if (*p == '.')
			break;

		if (*p == '0' && p0 == nullptr)
			p0 = p;
		if (*p != '0' && *p != '#')
			return false;

		p++;
	}
	int integralMinCount = p0 == nullptr ? 0 : p - p0;
	int fractionalMinCount = 0;
	int fractionalMaxCount = 0;
	p0 = nullptr;
	const wchar_t* pPoint = p;
	if (*p)
	{
		p++;
		while (*p)
		{
			if (*p != '0' && *p != '#')
				return false;

			if (*p == '0')
				p0 = p;

			p++;
		}

		fractionalMinCount = p0 == nullptr ? 0 : pPoint - p0;
		fractionalMaxCount = p - pPoint - 1;
	}

	double absVal = std::abs(value);
	// 四舍五入
	double pw = std::pow(10, fractionalMaxCount);
	absVal = (absVal * pw + 0.5) / pw;
	// 取整数部分和小数部分
	int integralPart = (int)absVal;
	double fractionalPart = absVal - integralPart;
	output.clear();

	// 转换整数部分
	while (integralPart > 0)
	{
		int digit = integralPart % 10;
		integralPart = integralPart / 10;
		output.push_back((wchar_t)(digit + '0'));
	}
	if (output.size() < (size_t)integralMinCount)
		output.append(integralMinCount - (int)output.size(), '0');
	if (value < 0)
		output.push_back('-');
	std::reverse(output.begin(), output.end());

	// 转换小数部分
	if (fractionalMaxCount <= 0)
		return true;

	output.push_back('.');
	fractionalMinCount += (int)output.size();
	for (int i = 0; i < fractionalMaxCount; ++i)
	{
		int digit = (int)(fractionalPart * 10.0);
		fractionalPart = fractionalPart * 10.0 - digit;
		assert(digit < 10);
		output.push_back((wchar_t)(digit + '0'));
	}
	// trim
	while (output.back() == '0' && output.size() > (size_t)fractionalMinCount)
		output.pop_back();

	return true;
}

static wchar_t _hexToNum(wchar_t ch)
{
	if (ch <= '9')
		return ch - '0';
	else if (ch <= 'F')
		return ch - 'A';
	else
		return ch - 'a';
}

static void replaceEscapeChar(const wchar_t* &pSrc, const wchar_t* pEnd, wchar_t* &pDst)
{
	wchar_t dstChar = 0;
	int charCount = 0;

	switch (*pSrc)
	{
	case 'a':
		dstChar = '\a';
		break;
	case 'b':
		dstChar = '\b';
		break;
	case 'f':
		dstChar = '\f';
		break;
	case 'n':
		dstChar = '\n';
		break;
	case 'r':
		dstChar = '\r';
		break;
	case 't':
		dstChar = '\t';
		break;
	case 'v':
		dstChar = '\v';
		break;
	case '0':
		while (pSrc < pEnd && isOctDigit(*pSrc) && charCount < 3)
		{
			dstChar = dstChar * 8 + (*pSrc - '0');
			++pSrc;
			++charCount;
		}
		--pSrc;
		break;
	case 'x':
		++pSrc;
		while (pSrc < pEnd && isHexDigit(*pSrc) && charCount < 2)
		{
			dstChar = dstChar * 16 + _hexToNum(*pSrc);
			++pSrc;
			++charCount;
		}
		--pSrc;
		break;
	case 'u':
		++pSrc;
		while (pSrc < pEnd && isHexDigit(*pSrc) && charCount < 4)
		{
			dstChar = dstChar * 16 + _hexToNum(*pSrc);
			++pSrc;
			++charCount;
		}
		--pSrc;
		break;
	default:
		dstChar = *pSrc;
		break;
	}

	++pSrc;
	*pDst++ = dstChar;
}

size_t xiyue::xiyue_unescapeCppStyleInplace(wchar_t* str, size_t len)
{
	const wchar_t* pSrc = str;
	const wchar_t* pEnd = str + len;
	size_t resultLen;
	// 先查找第一个出现转义字符的地方
	while (pSrc < pEnd)
	{
		if (*pSrc == '\\')
			break;
		++pSrc;
	}
	resultLen = pSrc - str;
	if (pSrc >= pEnd)
		return resultLen;
	// 开始转义替换
	wchar_t* pDst = const_cast<wchar_t*>(pSrc);
	do 
	{
		pSrc++;
		// 替换转义字符
		replaceEscapeChar(pSrc, pEnd, pDst);
		++resultLen;
		// 搜索下一个转义字符
		while (pSrc < pEnd && *pSrc != '/')
		{
			*pDst++ = *pSrc++;
			++resultLen;
		}
	} while (pSrc < pEnd);

	return resultLen;
}

wstring xiyue::xiyue_escapeCppStyleChars(const wchar_t* str, size_t len)
{
	const wchar_t* p = str;
	const wchar_t* pEnd = str + len;
	wstring result;

	while (p < pEnd)
	{
		wchar_t ch = *p++;
		switch (ch)
		{
		case '\t':
			result.append(L"\\t");
			break;
		case '\r':
			result.append(L"\\r");
			break;
		case '\n':
			result.append(L"\\n");
			break;
		case '"':
			result.append(L"\\\"");
			break;
		case '\\':
			result.append(L"\\\\");
			break;
		case '\'':
			result.append(L"\\'");
			break;
		case '\a':
			result.append(L"\\a");
			break;
		case '\b':
			result.append(L"\\b");
			break;
		case '\f':
			result.append(L"\\f");
			break;
		case '\0':
			result.append(L"\\0");
			break;
		default:
			if (!isAscii(ch))
			{
				wchar_t buffer[16];
				result.append(L"\\u").append(itow(buffer, ch, 16));
			}
			else
			{
				result.push_back(ch);
			}
			break;
		}
	}

	return result;
}

/*
	状态集：
	0: 跳过开头非字母符号，遇到字符跳到状态1，否则跳到状态3
	1: 读入一个单词，即字母数字的集合体，遇到非单词字符跳到状态2，否则跳到状态3
	2: 跳过连续n个非单词字符，遇到单词字符之后，尝试转换大写，然后跳到状态1，否则跳到状态3
	3: 结束
*/
ConstString xiyue::xiyue_makeCamelCaseName(const ConstString& src, bool uppercaseFirstLetter)
{
	ConstString result = ConstString::makeByReservedSize(src.length());
	wchar_t* pDst = result._getStringData()->stringData();
	const wchar_t* pSrc = src.data();

	int state = 0;
	for (;;)
	{
		wchar_t ch = *pSrc;
		switch (state)
		{
		case 0:
			if (ch == 0)
			{
				state = 3;
			}
			else if (isAlpha(ch))
			{
				*pDst++ = static_cast<wchar_t>(uppercaseFirstLetter ? toupper(ch) : tolower(ch));
				state = 1;
			}
			break;
		case 1:
			if (isAlphaDigit(ch))
				*pDst++ = static_cast<wchar_t>(tolower(ch));
			else if (ch == 0)
				state = 3;
			else
				state = 2;
			break;
		case 2:
			if (ch == 0)
			{
				state = 3;
			}
			else if (isAlphaDigit(ch))
			{
				*pDst++ = static_cast<wchar_t>(toupper(ch));
				state = 1;
			}
			break;
		}

		if (state == 3)
			break;

		pSrc++;
	}

	*pDst = 0;
	result._resetLength(pDst - result._getStringData()->stringData());
	assert(result._getStringData()->nBufferSize >= result.length());
	return result;
}

bool ParseUtils::matchString(ConstStringPointer& p, bool* hasEscapeChar, bool* hasUnmatchedQuote)
{
	bool _hasEscapedChar = false;
	bool _hasUnmatchedQuote = true;
	wchar_t quote = *p;

	if (quote != '"' && quote != '\'')
		return false;

	++p;

	wchar_t ch = *p;
	while (ch)
	{
		if (ch == quote)
		{
			++p;
			_hasUnmatchedQuote = false;
			break;
		}

		if (ch == '\\')
		{
			_hasEscapedChar = true;
			++p;
			ch = *p;
			if (ch == 0)
				break;
		}

		++p;
		ch = *p;
	}

	if (hasEscapeChar)
		*hasEscapeChar = _hasEscapedChar;
	if (hasUnmatchedQuote)
		*hasUnmatchedQuote = _hasUnmatchedQuote;

	return true;
}

bool ParseUtils::matchID(ConstStringPointer& p)
{
	wchar_t ch = *p;
	if (!isAlpha(ch) && ch != '_')
		return false;

	do 
	{
		++p;
		ch = *p;
	} while (isWordChar(ch));

	return true;
}

/*
	number = [-+]? '\d'+
	number = [-+]? ('\d'+)? '.' '\d'+ ([eE] [-+]? '\d'+)?
	number = [-+]? '\d'+ '.' ('\d'+)? ([eE] [-+]? '\d'+)?

	NFA 为：
	digraph number {
		rankdir = LR
		0->1
		0->1 [label=s]
		1->2 [label=d]
		2->3
		2->2 [label=d]
		3->4
		3->10 [label="."]
		1->4
		4->5 [label="."]
		5->6 [label=d]
		6->6 [label=d]
		6->7 [label=e]
		6->12
		3->12
		7->8 [label=s]
		7->8
		8->9 [label=d]
		9->9 [label=d]
		9->12
		10->11
		11->12
		11->7 [label=e]
	}

	DFA 为：
	digraph dfa {
		rankdir = LR
		2 [shape=doublecircle]
		4 [shape=doublecircle]
		5 [shape=doublecircle]
		8 [shape=doublecircle]
		0->1 [label=s]
		0->2 [label=d]
		0->3 [label="."]
		1->2 [label=d]
		1->3 [label="."]
		2->2 [label=d]
		2->4 [label="."]
		3->5 [label=d]
		4->5 [label=d]
		4->6 [label=e]
		5->5 [label=d]
		5->6 [label=e]
		6->7 [label=s]
		7->8 [label=d]
		8->8 [label=d]
	}

	抽象四种输入符号：
	s = 0 = [-+]
	d = 1 = [0-9]
	e = 2 = [eE]
	. = 3 = [.]

	所以，DFA 为：

		s	d	e	.
	0	1	2	-	3
	1	-	2	-	3
	2	-	2	-	4
	3	-	5	-	-
	4	-	5	6	-
	5	-	5	6	-
	6	7	-	-	-
	7	-	8	-	-
	8	-	8	-	-

	因为 0 只能是初始状态，所以跳转状态中，0 表示出错

	此外，2 4 5 8 是接受状态
*/

static const uint8_t g_numberMatchDFA[8][4] = {
	{ 1, 2, 0, 3 },
	{ 0, 2, 0, 3 },
	{ 0, 2, 0, 4 },
	{ 0, 5, 0, 0 },
	{ 0, 5, 6, 0 },
	{ 7, 0, 0, 0 },
	{ 0, 8, 0, 0 },
	{ 0, 8, 0, 0 }
};

static int transformNumberDfaInput(wchar_t ch)
{
	switch (ch)
	{
	case '.':
		return 3;
	case '+':
	case '-':
		return 0;
	case 'e': case 'E':
		return 2;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		return 1;
	default:
		return -1;
	}
}

bool ParseUtils::matchNumber(ConstStringPointer& p2, bool* isInteger)
{
	ConstStringPointer p = p2;
	wchar_t ch = *p;
	bool _isInteger = true;
	int state = 0;
	int lastSuccPos = p.getOffset();

	for (;;)
	{
		int input = transformNumberDfaInput(ch);
		if (input == -1)
			break;

		if (input == '2' || input == '3')
			_isInteger = false;

		state = g_numberMatchDFA[state][input];
		if (state == 0)
			break;

		++p;
		if (state == 2 || state == 4 || state == 5 || state == 8)
			lastSuccPos = p.getOffset();

		ch = *p;
	}

	if (lastSuccPos == p2.getOffset())
		return false;

	if (isInteger)
		*isInteger = _isInteger;

	p2.reset(lastSuccPos);
	return true;
}

bool ParseUtils::matchCppStyleComment(ConstStringPointer& p, bool* isBlockComment, bool* isBlockCommentUnmatched)
{
	bool _isBlockComment;
	bool _isBlockCommentUnmatched = true;

	if (*p != '/')
		return false;

	wchar_t ch = *(p + 1);
	if (ch == '/')
		_isBlockComment = false;
	else if (ch == '*')
		_isBlockComment = true;
	else
		return false;

	p += 2;

	ch = *p;
	while (ch)
	{
		if (_isBlockComment)
		{
			if (ch == '*' && *(p + 1) == '/')
			{
				p += 2;
				_isBlockCommentUnmatched = false;
				break;
			}
		}
		else
		{
			if (isLineBreak(ch))
				break;
		}

		++p;
		ch = *p;
	}

	if (isBlockComment)
		*isBlockComment = _isBlockComment;

	if (isBlockCommentUnmatched && isBlockComment)
		*isBlockCommentUnmatched = _isBlockCommentUnmatched;

	return true;
}

bool ParseUtils::matchWhiteSpace(ConstStringPointer& p)
{
	if (!isSpace(*p))
		return false;

	do 
	{
		++p;
	} while (isSpace(*p));

	return true;
}
