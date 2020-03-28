#include "stdafx.h"
#include "xiyue_json_lexer.h"
#include "xiyue_string_format.h"
#include "xiyue_json_exception.h"
#include "xiyue_encoding.h"

using namespace std;
using namespace xiyue;

static uint8_t g_jsonTokenMap[128] = { 0 };

class JsonLexerCharMapInitializer
{
public:
	JsonLexerCharMapInitializer() {
		g_jsonTokenMap['{'] = JsonToken_lBrace;
		g_jsonTokenMap['}'] = JsonToken_rBrace;
		g_jsonTokenMap['['] = JsonToken_lBracket;
		g_jsonTokenMap[']'] = JsonToken_rBracket;
		g_jsonTokenMap[','] = JsonToken_comma;
		g_jsonTokenMap[':'] = JsonToken_colon;
		g_jsonTokenMap['"'] = JsonToken_string;
		g_jsonTokenMap['\''] = JsonToken_string;
		for (int i = '0'; i <= '9'; ++i)
			g_jsonTokenMap[i] = JsonToken_int;
		g_jsonTokenMap['-'] = JsonToken_int;
		g_jsonTokenMap['+'] = JsonToken_int;
		for (int i = 'a'; i <= 'z'; ++i)
			g_jsonTokenMap[i] = JsonToken_id;
		for (int i = 'A'; i <= 'Z'; ++i)
			g_jsonTokenMap[i] = JsonToken_id;
		g_jsonTokenMap['_'] = JsonToken_id;
		g_jsonTokenMap['/'] = JsonToken_comment;
	}
};

static JsonLexerCharMapInitializer g_initializer;

JsonLexer::JsonLexer(ConstString jsonText)
	: m_text(jsonText)
	, m_cursor(&m_text)
{
	m_token.tokenType = JsonToken_unknown;
}

void JsonLexer::parseToken()
{
	// skip ws and comments
	skipTrivialTokens();

	if (*m_cursor == 0)
	{
		m_token.fill(JsonToken_EOF, m_cursor.getOffset(), 1);
		return;
	}

	bool flag;
	uint32_t startPos = m_cursor.getOffset();
	ConstString tokenString;
	JsonTokenType tokenType = static_cast<JsonTokenType>(g_jsonTokenMap[*m_cursor]);
	switch (tokenType)
	{
	case JsonToken_unknown:
		m_token.fill(JsonToken_unknown, startPos, 1);
		++m_cursor;
		throw JsonException(JsonError_unexpectedToken, startPos, 1);
	case JsonToken_string:
		ParseUtils::matchString(m_cursor, &m_token.isEscapedString, &flag);
		if (flag)
			throw JsonException(JsonError_unclosedString, startPos, m_cursor.getOffset() - startPos);
		m_token.fill(tokenType, startPos, m_cursor.getOffset() - startPos);
		break;
	case JsonToken_int:
		if (!ParseUtils::matchNumber(m_cursor, &flag))
			throw JsonException(JsonError_unexpectedToken, startPos, 1);
		if (flag)
			m_token.fill(JsonToken_int, startPos, m_cursor.getOffset() - startPos);
		else
			m_token.fill(JsonToken_real, startPos, m_cursor.getOffset() - startPos);
		break;
	case JsonToken_id:
		ParseUtils::matchID(m_cursor);
		tokenString = m_text.substr(startPos, m_cursor.getOffset() - startPos);
		if (tokenString == L"null"_cs)
		{
			m_token.fill(JsonToken_null, startPos, m_cursor.getOffset() - startPos);
		}
		else if (tokenString == L"true"_cs)
		{
			m_token.fill(JsonToken_boolean, startPos, m_cursor.getOffset() - startPos);
			m_token.isTrueString = true;
		}
		else if (tokenString == L"false"_cs)
		{
			m_token.fill(JsonToken_boolean, startPos, m_cursor.getOffset() - startPos);
			m_token.isTrueString = false;
		}
		else
		{
			throw JsonException(JsonError_unexpectedToken, startPos, m_cursor.getOffset() - startPos);
		}
		break;
	default:
		m_token.fill(tokenType, startPos, 1);
		++m_cursor;
		break;
	}
}

void JsonLexer::skipTrivialTokens()
{
	// Ìø¹ý¿Õ°××Ö·ûºÍ×¢ÊÍ
	wchar_t ch = *m_cursor;
	for (;;)
	{
		if (isSpace(ch))
		{
			++m_cursor;
			ch = *m_cursor;
			continue;
		}

		if (ch == '/')
		{
			bool isBlockQuoteUnmatched = false;
			int startPos = m_cursor.getOffset();
			if (ParseUtils::matchCppStyleComment(m_cursor, nullptr, &isBlockQuoteUnmatched))
				continue;

			if (isBlockQuoteUnmatched)
				throw JsonException(JsonError_unclosedComment, startPos, m_cursor.getOffset() - startPos);
		}

		break;
	}
}

const JsonToken& JsonLexer::peekToken()
{
	if (m_token.tokenType != JsonToken_unknown)
		return m_token;

	parseToken();
	m_cursor.reset(m_token.offset);
	return m_token;
}

bool JsonLexer::consumeToken()
{
	if (m_token.tokenType == JsonToken_unknown)
		peekToken();

	if (m_token.tokenType == JsonToken_EOF)
		return false;

	m_cursor.reset(m_token.offset + m_token.length);
	m_token.tokenType = JsonToken_unknown;
	return true;
}
