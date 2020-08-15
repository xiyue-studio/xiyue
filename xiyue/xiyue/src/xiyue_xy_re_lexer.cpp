#include "stdafx.h"
#include "xiyue_xy_re_lexer.h"
#include "xiyue_encoding.h"

using namespace std;
using namespace xiyue;

#define NORMAL_MODE 0
#define CLASS_MODE 1

XyReLexer::XyReLexer(const wchar_t* strBegin, const wchar_t* strEnd)
{
	m_begin = strBegin;
	m_end = strEnd;
	m_cursor = m_begin;
	m_token.type = XyReToken_none;
	m_mode = NORMAL_MODE;
	m_isLooseMode = false;
}

const XyReToken* XyReLexer::peekToken()
{
	while (m_token.type == XyReToken_none || m_token.type == XyReToken_comment)
		parseToken();
	
	return &m_token;
}

void XyReLexer::consume()
{
	if (m_token.type != XyReToken_EOF)
		m_token.type = XyReToken_none;
}

void XyReLexer::parseToken()
{
	switch (m_mode)
	{
	case NORMAL_MODE:
		parseNormalModeToken();
		break;
	case CLASS_MODE:
		parseClassModeToken();
		break;
	default:
		assert(!"Unknown mode");
		break;
	}
}

void XyReLexer::parseNormalModeToken()
{
	// 如果是松散模式 x，则跳过所有的空白字符
	if (m_isLooseMode)
		skipWS();

	m_token.cursor = m_cursor;
	m_token.length = 1;

	switch (getChar())
	{
	case 0:
		m_token.type = XyReToken_EOF;
		return;
	case '|':
		m_token.type = XyReToken_pipe;
		++m_cursor;
		return;
	case '*':
		++m_cursor;
		if (getChar() == '?')
		{
			m_token.type = XyReToken_starQuestion;
			++m_cursor;
			++m_token.length;
		}
		else if (getChar() == '+')
		{
			m_token.type = XyReToken_starPlus;
			++m_cursor;
			++m_token.length;
		}
		else
		{
			m_token.type = XyReToken_star;
		}
		return;
	case '?':
		++m_cursor;
		if (getChar() == '?')
		{
			m_token.type = XyReToken_questionQuestion;
			++m_cursor;
			++m_token.length;
		}
		else if (getChar() == '+')
		{
			m_token.type = XyReToken_questionPlus;
			++m_cursor;
			++m_token.length;
		}
		else
		{
			m_token.type = XyReToken_question;
		}
		return;
	case '+':
		++m_cursor;
		if (getChar() == '?')
		{
			m_token.type = XyReToken_plusQuestion;
			++m_cursor;
			++m_token.length;
		}
		else if (getChar() == '+')
		{
			m_token.type = XyReToken_plusPlus;
			++m_cursor;
			++m_token.length;
		}
		else
		{
			m_token.type = XyReToken_plus;
		}
		return;
	case '{':
		++m_cursor;
		if (!parseRepetitionToken())
		{
			m_token.arg1 = '{';
			m_token.type = XyReToken_char;
		}
		return;
	case '\\':
		parseEscapeToken();
		return;
	case '.':
		++m_cursor;
		m_token.type = XyReToken_dot;
		return;
	case '^':
		++m_cursor;
		m_token.type = XyReToken_caret;
		return;
	case '$':
		++m_cursor;
		m_token.type = XyReToken_dollar;
		return;
	case '[':
		++m_cursor;
		if (getChar() == '^')
		{
			++m_cursor;
			++m_token.length;
			m_token.type = XyReToken_lBracketCaret;
		}
		else
		{
			m_token.type = XyReToken_lBracket;
		}
		m_mode = CLASS_MODE;
		return;
	case '(':
		parseLParenToken();
		return;
	case ')':
		++m_cursor;
		m_token.type = XyReToken_rParen;
		return;
	case '#':
		++m_cursor;
		if (m_isLooseMode)
		{
			parseLineComment();
			return;
		}
		break;
	}

	// 剩下的，均按照普通字符处理
	m_token.arg1 = (int)getChar();
	++m_cursor;
	m_token.type = XyReToken_char;
}

void XyReLexer::skipWS()
{
	while (isSpace(getChar()))
	{
		++m_cursor;
	}
}

bool XyReLexer::parseRepetitionToken()
{
	const wchar_t* location = m_cursor;
	m_token.type = XyReToken_none;

	wchar_t ch = getChar();
	m_token.arg1 = -1;
	m_token.arg2 = -1;
	if (isDigit(ch))
	{
		m_token.arg1 = ch - '0';
		++m_cursor;
		while (isDigit(getChar()))
		{
			m_token.arg1 = m_token.arg1 * 10 + ch - '0';
			++m_cursor;
		}
		ch = getChar();
	}
	else if (ch != ',')
	{
		return false;
	}

	if (ch == ',')
	{
		++m_cursor;
		ch = getChar();
		if (isDigit(ch))
		{
			m_token.arg2 = ch - '0';
			++m_cursor;
			while (isDigit(getChar()))
			{
				m_token.arg2 = m_token.arg2 * 10 + getChar() - '0';
				++m_cursor;
			}
			ch = getChar();
		}
		else if (m_token.arg1 == -1)
		{
			m_cursor = location;
			return false;
		}
	}
	else
	{
		m_token.type = XyReToken_repeat;
	}

	if (ch != '}')
	{
		m_cursor = location;
		return false;
	}

	++m_cursor;
	ch = getChar();
	if (ch == '?' || ch == '+')
		++m_cursor;
	else
		ch = 0;

	m_token.length = m_cursor - m_token.cursor;
	if (m_token.type == XyReToken_repeat)
		return true;

	if (m_token.arg1 == -1)
		m_token.arg1 = 0;
	m_token.type = ch == '?' ? XyReToken_rangeQuestion : (
		ch == '+' ? XyReToken_rangePlus : XyReToken_range
		);
	return true;
}

void XyReLexer::parseEscapeToken()
{
	++m_cursor;
	wchar_t ch = getChar();
	m_token.length = 2;
	++m_cursor;
	switch (ch)
	{
	case 'f':
		m_token.type = XyReToken_char;
		m_token.arg1 = '\f';
		return;
	case 'n':
		m_token.type = XyReToken_char;
		m_token.arg1 = '\n';
		return;
	case 'r':
		m_token.type = XyReToken_char;
		m_token.arg1 = '\r';
		return;
	case 's':
		m_token.type = XyReToken_space;
		return;
	case 'S':
		m_token.type = XyReToken_nonSpace;
		return;
	case 't':
		m_token.type = XyReToken_char;
		m_token.arg1 = '\t';
		return;
	case 'v':
		m_token.type = XyReToken_char;
		m_token.arg1 = '\v';
		return;
	case 'b':
		m_token.type = m_mode == CLASS_MODE ? XyReToken_char : XyReToken_boundary;
		m_token.arg1 = 'b';
		return;
	case 'B':
		m_token.type = m_mode == CLASS_MODE ? XyReToken_char : XyReToken_nonBoundary;
		m_token.arg1 = 'B';
		return;
	case 'd':
		m_token.type = XyReToken_digit;
		return;
	case 'D':
		m_token.type = XyReToken_nonDigit;
		return;
	case 'w':
		m_token.type = XyReToken_word;
		return;
	case 'W':
		m_token.type = XyReToken_nonWord;
		return;
	case 'G':
		m_token.type = m_mode == CLASS_MODE ? XyReToken_char : XyReToken_lastPosMatch;
		m_token.arg1 = 'G';
		return;
	case '0':
		m_token.type = XyReToken_char;
		m_token.arg1 = 0;
		while (isOctDigit(getChar()) && m_token.length < 5)
		{
			m_token.arg1 = m_token.arg1 * 8 + getChar() - '0';
			++m_cursor;
			++m_token.length;
		}
		return;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		if (m_mode == CLASS_MODE)
		{
			m_token.type = XyReToken_char;
			m_token.arg1 = ch;
		}
		else
		{
			m_token.type = XyReToken_backReference;
			m_token.arg1 = ch - '0';
		}
		return;
	case 'x':
		m_token.type = XyReToken_char;
		if (!isHexDigit(getChar()) || !isHexDigit(getCharAt(m_cursor + 1)))
		{
			m_token.arg1 = 'x';
		}
		else
		{
			m_token.arg1 = hexToInt(*m_cursor) * 16 + hexToInt(*(m_cursor + 1));
			m_cursor += 2;
			m_token.length += 2;
		}
		return;
	case 'u':
		m_token.type = XyReToken_char;
		if (!isHexDigit(getChar()) || !isHexDigit(getCharAt(m_cursor + 1)) || !isHexDigit(getCharAt(m_cursor + 2)) || !isHexDigit(getCharAt(m_cursor + 3)))
		{
			m_token.arg1 = 'u';
		}
		else
		{
			m_token.arg1 = hexToInt(*m_cursor) * 16 * 16 * 16 + hexToInt(*(m_cursor + 1)) * 16 * 16 + hexToInt(*(m_cursor + 2)) * 16 + hexToInt(*(m_cursor + 3));
			m_cursor += 4;
			m_token.length += 4;
		}
		return;
	case 'k':
		if (m_mode != CLASS_MODE && parseReference('<', '>'))
		{
			m_token.type = XyReToken_nameReference;
		}
		else
		{
			m_token.type = XyReToken_char;
			m_token.arg1 = 'k';
		}
		return;
	case 'p':
		if (m_mode != CLASS_MODE && parseReference('{', '}'))
		{
			m_token.type = XyReToken_unicodeProperty;
		}
		else
		{
			m_token.type = XyReToken_char;
			m_token.arg1 = 'p';
		}
		return;
	default:
		m_token.type = XyReToken_char;
		m_token.arg1 = ch;
		return;
	}
}

bool XyReLexer::parseReference(wchar_t start, wchar_t end)
{
	if (getChar() != start)
		return false;

	const wchar_t* location = m_cursor;
	++m_cursor;
	if (m_isLooseMode)
		skipWS();

	m_token.arg1 = m_cursor - m_token.cursor;
	if (!isWordChar(getChar()))
	{
		m_cursor = location;
		return false;
	}

	while (isWordChar(getChar()))
	{
		++m_cursor;
	}
	m_token.arg2 = m_cursor - m_token.cursor - m_token.arg1;
	if (m_isLooseMode)
		skipWS();

	if (getChar() != end)
	{
		m_cursor = location;
		return false;
	}

	++m_cursor;
	m_token.length = m_cursor - m_token.cursor;
	return true;
}

void XyReLexer::parseLParenToken()
{
	++m_cursor;

	wchar_t ch = getChar();
	if (ch != '?')
	{
		m_token.type = XyReToken_lParen;
		m_token.length = 1;
		return;
	}

	++m_cursor;
	m_token.length = 2;
	ch = getChar();
	switch (ch)
	{
	case '<':
		if (getCharAt(m_cursor + 1) == '=')
		{
			m_token.type = XyReToken_lLookAheadParen;
			m_token.length = 4;
			m_cursor += 2;
		}
		else if (getCharAt(m_cursor + 1) == '!')
		{
			m_token.type = XyReToken_lLookAheadNegParen;
			m_token.length = 4;
			m_cursor += 2;
		}
		else if (parseReference('<', '>'))
		{
			m_token.type = XyReToken_lNamedCaptureParen;
		}
		else
		{
			m_token.type = XyReToken_lNonCaptureParen;
		}
		return;
	case '>':
		++m_cursor;
		m_token.type = XyReToken_lFixedParen;
		m_token.length = 3;
		return;
	case ':':
		++m_cursor;
		m_token.type = XyReToken_lNonCaptureParen;
		m_token.length = 3;
		return;
	case '=':
		++m_cursor;
		m_token.type = XyReToken_lLookBehindParen;
		m_token.length = 3;
		return;
	case '!':
		++m_cursor;
		m_token.type = XyReToken_lLookBehindNegParen;
		m_token.length = 3;
		return;
	case 'i':
	case 'm':
	case 'x':
		++m_cursor;
		m_token.arg1 = ch;
		if (getChar() == ':')
		{
			++m_cursor;
			m_token.type = XyReToken_lEmbeddedSwitchParen;
			m_token.length = 4;
		}
		else if (getChar() == ')')
		{
			++m_cursor;
			m_token.type = XyReToken_lSwitchOnParen;
			m_token.length = 4;
		}
		else
		{
			--m_cursor;
			m_token.type = XyReToken_lNonCaptureParen;
			m_token.length = 2;
		}
		return;
	case '-':
		ch = getCharAt(m_cursor + 1);
		if ((ch == 'i' || ch == 'm' || ch == 'x') && getCharAt(m_cursor + 2) == ')')
		{
			m_token.type = XyReToken_lSwitchOffParen;
			m_token.length = 5;
			m_token.arg1 = ch;
			m_cursor += 3;
		}
		else
		{
			m_token.type = XyReToken_lNonCaptureParen;
			m_token.length = 2;
		}
		return;
	case '#':
		parseComment();
		return;
	default:
		m_token.type = XyReToken_lNonCaptureParen;
		m_token.length = 2;
		return;
	}
}

void XyReLexer::parseComment()
{
	++m_cursor;
	wchar_t ch = getChar();
	while (ch)
	{
		if (ch == ')')
		{
			++m_cursor;
			break;
		}

		++m_cursor;
		ch = getChar();
	}

	m_token.type = XyReToken_comment;
	m_token.length = m_cursor - m_token.cursor;
}

void XyReLexer::parseLineComment()
{
	wchar_t ch = getChar();
	while (ch && !isLineBreak(ch))
	{
		++m_cursor;
	}

	m_token.type = XyReToken_comment;
	m_token.length = m_cursor - m_token.cursor;
}

void XyReLexer::parseClassModeToken()
{
	m_token.cursor = m_cursor;
	wchar_t ch = getChar();
	
	if (ch == 0)
	{
		throw exception("Unexpected EOF");
	}

	if (ch == ']')
	{
		++m_cursor;
		m_token.type = XyReToken_rBracket;
		m_token.length = 1;
		m_mode = NORMAL_MODE;
		return;
	}

	int ch1 = 0;
	if (ch == '\\')
	{
		parseEscapeToken();
		if (m_token.type != XyReToken_char)
			return;

		ch1 = m_token.arg1;
	}
	else
	{
		ch1 = ch;
		++m_cursor;
		m_token.type = XyReToken_char;
		m_token.length = 1;
		m_token.arg1 = ch;
	}

	const wchar_t* p = m_cursor;
	ch = getChar();
	if (ch != '-')
		return;

	ch = getCharAt(m_cursor + 1);
	if (ch == ']' || ch == 0)
		return;
	++m_cursor;
	if (ch == '\\')
	{
		const wchar_t* tokenCursor = m_token.cursor;
		m_token.cursor = m_cursor;
		parseEscapeToken();
		m_token.cursor = tokenCursor;
		if (m_token.type != XyReToken_char)
		{
			m_cursor = p;
			m_token.length = p - tokenCursor;
			m_token.type = XyReToken_char;
			m_token.arg1 = ch1;
		}
		else
		{
			m_token.arg2 = m_token.arg1;
			m_token.length = m_cursor - tokenCursor;
			m_token.type = XyReToken_charRange;
			m_token.arg1 = ch1;
		}
	}
	else
	{
		++m_cursor;
		m_token.arg1 = ch1;
		m_token.type = XyReToken_charRange;
		m_token.arg2 = ch;
		m_token.length = m_cursor - m_token.cursor;
	}
}
