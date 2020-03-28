#pragma once
#include "xiyue_const_string.h"

namespace xiyue
{
	enum JsonTokenType : uint8_t
	{
		JsonToken_unknown = 0,
		JsonToken_lBrace = 1,         ///< {
		JsonToken_rBrace = 2,         ///< }
		JsonToken_lBracket = 3,       ///< [
		JsonToken_rBracket = 4,       ///< ]
		JsonToken_comma = 5,          ///< ,
		JsonToken_colon = 6,          ///< :
		JsonToken_int = 7,            ///< INT
		JsonToken_real = 8,           ///< REAL
		JsonToken_string = 9,         ///< STRING
		JsonToken_boolean = 10,       ///< BOOLEAN
		JsonToken_null = 11,          ///< NULL
		JsonToken_EOF = 12,           ///< EOF
		JsonToken_comment = 98,
		JsonToken_id = 99
	};

	struct JsonToken
	{
		JsonTokenType tokenType;
		union {
			bool isEscapedString;
			bool isTrueString;
		};
		uint32_t offset;
		uint32_t length;

		inline void fill(JsonTokenType type, uint32_t os, uint32_t len) {
			tokenType = type;
			offset = os;
			length = len;
		}
	};

	class JsonLexer
	{
	public:
		explicit JsonLexer(ConstString jsonText);

	public:
		const JsonToken& peekToken();
		bool consumeToken();

	protected:
		void parseToken();
		void skipTrivialTokens();

	private:
		JsonToken m_token;
		ConstString m_text;
		ConstStringPointer m_cursor;
	};
}
