#pragma once

namespace xiyue
{
	enum XyReTokenType
	{
		XyReToken_none = 0,
		XyReToken_EOF,							///< EOF
		XyReToken_pipe,							///< |
		XyReToken_star,							///< *
		XyReToken_plus,							///< +
		XyReToken_question,						///< ?
		XyReToken_starQuestion,					///< *?
		XyReToken_plusQuestion,					///< +?
		XyReToken_questionQuestion,				///< ??
		XyReToken_starPlus,						///< *+
		XyReToken_plusPlus,						///< ++
		XyReToken_questionPlus,					///< ?+
		XyReToken_repeat,						///< {n}?+
		XyReToken_range,						///< {n,m}
		XyReToken_rangeQuestion,				///< {n,m}?
		XyReToken_rangePlus,					///< {n,m}+
		XyReToken_char,							///< A
		XyReToken_formFeed,						///< \f
		XyReToken_newLine,						///< \n
		XyReToken_carriageReturn,				///< \r
		XyReToken_space,						///< \s
		XyReToken_nonSpace,						///< \S
		XyReToken_tab,							///< \t
		XyReToken_verticleTab,					///< \v
		XyReToken_boundary,						///< \b
		XyReToken_nonBoundary,					///< \B
		XyReToken_digit,						///< \d
		XyReToken_nonDigit,						///< \D
		XyReToken_word,							///< \w
		XyReToken_nonWord,						///< \W
		XyReToken_lastPosMatch,					///< \G
		XyReToken_backReference,				///< \1
		XyReToken_escapeChar,					///< \(
		XyReToken_nameReference,				///< \k<name>
		XyReToken_unicodeProperty,				///< \p{property}
		XyReToken_dot,							///< .
		XyReToken_caret,						///< ^
		XyReToken_dollar,						///< $
		XyReToken_lBracket,						///< [
		XyReToken_rBracket,						///< ]
		XyReToken_lBracketCaret,				///< [^
		XyReToken_charRange,					///< a-z
		XyReToken_lParen,						///< (
		XyReToken_rParen,						///< )
		XyReToken_lFixedParen,					///< (?>
		XyReToken_lNonCaptureParen,				///< (?: 或 (?
		XyReToken_lLookAheadParen,				///< (?<=
		XyReToken_lLookAheadNegParen,			///< (?<!
		XyReToken_lLookBehindParen,				///< (?=
		XyReToken_lLookBehindNegParen,			///< (?!
		XyReToken_lNamedCaptureParen,			///< (?<name>
		XyReToken_lEmbeddedSwitchParen,			///< (?imx:
		XyReToken_lSwitchOnParen,				///< (?imx)
		XyReToken_lSwitchOffParen,				///< (?-imx)
		XyReToken_comment
	};

	struct XyReToken
	{
		XyReTokenType type;
		const wchar_t* cursor;
		int length;
		int arg1;
		int arg2;
	};

	/*
		解析正则表达式词法的时候，有以下情况需要特殊处理。

		* 在 [] 内部的 ., ^, $, | 均按照字符处理
		* 在 [] 内部最前面或者最后面的 - 按照字符处理
		* 在字符位置出现的 {, }, ',' 以及数字，均按照原本字符处理
		* \b \B \G 在 [] 内部表示字符本身
	*/
	class XyReLexer
	{
	public:
		XyReLexer(const wchar_t* strBegin, const wchar_t* strEnd);
		~XyReLexer() = default;

	public:
		const XyReToken* peekToken();
		void consume();

		inline void setLooseMode(bool on) { m_isLooseMode = on; }

	private:
		void parseToken();
		void parseNormalModeToken();
		void skipWS();
		bool parseRepetitionToken();
		void parseEscapeToken();
		void parseLParenToken();
		void parseComment();
		void parseLineComment();
		void parseClassModeToken();
		bool parseReference(wchar_t start, wchar_t end);

		inline wchar_t getChar() const { return m_cursor >= m_end ? 0 : *m_cursor; }
		inline wchar_t getCharAt(const wchar_t* p) const { return p > m_end ? 0 : *p; }

	private:
		XyReToken m_token;
		int m_mode;
		bool m_isLooseMode;
		const wchar_t* m_begin;
		const wchar_t* m_end;
		const wchar_t* m_cursor;
	};
}
