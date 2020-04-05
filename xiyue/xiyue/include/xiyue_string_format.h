#pragma once

namespace xiyue
{
	/*
		支持的格式：

		类似 0.00 这样的数字，
		0 表示这个地方输出数字，如果是 0 也输出 0，
		# 表示这个地方输出数字，如果是没必要的 0 则不输出。

		注意，输出结果会清空 output。
	*/
	bool xiyue_formatNumber(std::wstring& output, const wchar_t* formatStr, double value);

	class ParseUtils
	{
	public:
		// 匹配单引号或者双引号包裹的字符串，忽略被转义的引号
		static bool matchString(ConstStringPointer& p, bool* hasEscapeChar, bool* hasUnmatchedQuote);

		static bool matchID(ConstStringPointer& p);

		// 匹配整数或者浮点数，支持 1.23e-5 这种形式的科学计数法，忽略大小写
		static bool matchNumber(ConstStringPointer& p, bool* isInteger);

		static bool matchCppStyleComment(ConstStringPointer& p, bool* isBlockComment, bool* isBlockCommentUnmatched);

		static bool matchWhiteSpace(ConstStringPointer& p);
	};

	class CppStringEscaper
	{
	public:
		size_t unescapeInplace(const wchar_t* str, size_t len);

	private:
		wchar_t m_escapeMark;
	};

	size_t xiyue_unescapeCppStyleInplace(wchar_t* str, size_t len);
	std::wstring xiyue_escapeCppStyleChars(const wchar_t* str, size_t len);
	inline std::wstring xiyue_escapeCppStyleChars(const ConstString& str) {
		return xiyue_escapeCppStyleChars(str.data(), str.length());
	}

	/*
		将类似于 name_of_variable 的命名方式转化成 nameOfVariable 的形式。
		多个单词之前如果有多个分隔，例如 `name      of----case`，都会被合并。
		起始位置的非字母字符会被忽略。
	*/
	ConstString xiyue_makeCamelCaseName(const ConstString& src, bool uppercaseFirstLetter);
}
