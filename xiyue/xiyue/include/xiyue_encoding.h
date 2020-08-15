#pragma once

namespace xiyue
{
	constexpr const char* UTF8_BOM = "\xef\xbb\xbf";
	constexpr const char* UTF16_LE_BOM = "\xff\xfe";
	constexpr const char* UTF16_BE_BOM = "\xfe\xff";

	enum StringEncoding
	{
		StringEncoding_unspecifiedEncoding = 0,
		StringEncoding_utf8 = 1,
		StringEncoding_utf16LE = 2,
		StringEncoding_utf16BE = 3,
		StringEncoding_gb2312 = 4,
		StringEncoding_utf8WithBOM = 5,
		StringEncoding_utf16LEWithBOM = 6,
		StringEncoding_utf16BEWithBOM = 7
	};

	enum NewLineStyle
	{
		NewLineStyle_unspecified = 0,
		NewLineStyle_crlf,		// \r\n
		NewLineStyle_cr,		// \r
		NewLineStyle_lf			// \n
	};

	/*
		返回值表示转换后的空间的字节数。
	*/
	size_t xiyue_transformStringEncoding(const void* src, void* dst, size_t dstSize,
		StringEncoding srcEncoding, StringEncoding dstEncoding);

	void* xiyue_transformStringEncoding(const void* src, StringEncoding srcEncoding,
		StringEncoding dstEncoding);

	StringEncoding xiyue_getEncodingFromBOM(const void* bytes, size_t size);

	inline constexpr bool isAscii(int c) { return c < 0x80; }

	inline constexpr bool isLineBreak(int c) { return c == '\r' || c == '\n'; }

	inline constexpr bool isSpace(int c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

	inline constexpr bool isAlpha(int c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
	inline constexpr bool isDigit(int c) { return c >= '0' && c <= '9'; }
	inline constexpr bool isAlphaDigit(int c) { return isAlpha(c) || isDigit(c); }
	inline constexpr bool isWordChar(int c) { return isAlphaDigit(c) || c == '_'; }
	inline constexpr bool isHexDigit(int c) {
		return c >= '0' && c <= '9'
			|| c >= 'a' && c <= 'f'
			|| c >= 'A' && c <= 'F';
	}
	inline constexpr int hexToInt(wchar_t hex) {
		switch (hex)
		{
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			return (int)hex - '0';
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
			return (int)hex - 'a' + 10;
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
			return (int)hex - 'A' + 10;
		default:
			assert(!"Not a hex digit");
			return 0;
		}
	}
	inline constexpr bool isOctDigit(int c) { return c >= '0' && c <= '7'; }
	inline constexpr bool isLowerAlpha(int c) { return c >= 'a' && c <= 'z'; }
	inline constexpr bool isUpperAlpha(int c) { return c >= 'A' && c <= 'Z'; }
}
