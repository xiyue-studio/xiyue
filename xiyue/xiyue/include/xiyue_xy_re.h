#pragma once
#include "xiyue_const_string.h"

namespace xiyue
{
	class XyReSubMatch
	{
	public:
		XyReSubMatch(const ConstString& matchedString, uint32_t position);
		~XyReSubMatch() = default;

	public:
		ConstString getMatchedString() const;
		uint32_t getMatchedPosition() const;
		uint32_t getMatchedLength() const;
		bool isEmpty() const;

	public:
		operator ConstString() const { return getMatchedString(); }

	private:
		uint32_t m_matchedPosition;
		ConstString m_matchedString;

		friend class XyReMatchBuilder;
	};

	class XyReMatch
	{
	public:
		XyReMatch();
		XyReMatch(XyReMatch&& m);
		~XyReMatch() = default;

	public:
		uint32_t getGroupCount() const;
		uint32_t getUnnamedGroupCount() const;
		uint32_t getNamedGroupCount() const;

		const XyReSubMatch& getSubMatch(int index) const;
		const XyReSubMatch& getSubMatch(const ConstString& name) const;
		const XyReSubMatch& getSubMatch(const wchar_t* name) const;
		const XyReSubMatch& getSubMatch(const std::wstring& name) const;

		bool isReady() const;
		bool isSuccess() const;
		ConstString getMatchedString() const;
		uint32_t getMatchedPosition() const;
		uint32_t getMatchedLength() const;
		ConstString getPrefixString() const;
		ConstString getSuffixString() const;

		ConstString format(const wchar_t* formatStr) const;
		ConstString format(const ConstString& formatStr) const;

	public:
		const XyReSubMatch& operator[](int index) const { return getSubMatch(index); }
		const XyReSubMatch& operator[](const wchar_t* name) const { return getSubMatch(name); }
		const XyReSubMatch& operator[](const ConstString& name) const { return getSubMatch(name); }
		const XyReSubMatch& operator[](const std::wstring& name) const { return getSubMatch(name); }
		operator ConstString() const { return getMatchedString(); }

	private:
		uint8_t m_state;
		uint32_t m_matchedPosition;
		uint32_t m_matchedLength;
		ConstString m_originalString;
		std::vector<XyReSubMatch> m_unnamedGroups;
		std::unordered_map<ConstString, XyReSubMatch> m_namedGroups;

		friend class XyReMatchBuilder;
	};

	class XyRe
	{
	public:
		XyRe(const wchar_t* regStrBegin, const wchar_t* regStrEnd, const wchar_t* flags = nullptr);
		XyRe(const wchar_t* regStr, const wchar_t* flags = nullptr);
		XyRe(const ConstString& regStr, const wchar_t* flags = nullptr);
		virtual ~XyRe();

	public:
		bool match(const ConstString& str, XyReMatch* matchOut = nullptr);
		bool match(const wchar_t* str, XyReMatch* matchOut = nullptr);
		bool match(const wchar_t* begin, const wchar_t* end, XyReMatch* matchOut = nullptr);
		bool match(const std::wstring& str, XyReMatch* matchOut = nullptr);

		bool search(const ConstString& str, XyReMatch* matchOut, int startIndex = 0);
		bool search(const wchar_t* str, XyReMatch* matchOut, int startIndex = 0);
		bool search(const wchar_t* begin, const wchar_t* end, XyReMatch* matchOut, int startIndex = 0);
		bool search(const std::wstring& str, XyReMatch* matchOut, int startIndex = 0);

		std::vector<XyReMatch> search(const ConstString& str);
		std::vector<XyReMatch> search(const wchar_t* str);
		std::vector<XyReMatch> search(const wchar_t* begin, const wchar_t* end);
		std::vector<XyReMatch> search(const std::wstring& str);

		bool testMatch(const ConstString& str);
		bool testMatch(const std::wstring& str);
		bool testMatch(const wchar_t* str);
		bool testMatch(const wchar_t* begin, const wchar_t* end);
		uint32_t testSearch(const ConstString& str, int startIndex = 0);
		uint32_t testSearch(const std::wstring& str, int startIndex = 0);
		uint32_t testSearch(const wchar_t* str, int startIndex = 0);
		uint32_t testSearch(const wchar_t* begin, const wchar_t* end, int startIndex = 0);

		uint32_t replace(const ConstString& srcStr, const ConstString& replacePattern);

		void compile();

	public:
		/*
			设置匹配是否忽略大小写。
		*/
		void setIgnoreCaseMode(bool on);
		/*
			设置匹配是否是全局匹配。

			全局匹配在 search 和 replace 的时候，会搜索所有匹配的结果，
			而并非指找第一个匹配的结果。
		*/
		void setGlobalSearchMode(bool on);
		/*
			设置 . 元字符是否也可以匹配换行符。
		*/
		void setDotMatchNewLine(bool on);
		/*
			设置 ^ 和 $ 仅匹配字符串的开头的结尾，还是匹配行的开头和结尾。

			on 表示可以匹配行头和行尾。
		*/
		void setMultiLineMode(bool on);
		/*
			设置是否支持松散模式，松散模式下，可以随意添加空格，并可以对
			正则表达式进行注释。
		*/
		void setLooseMode(bool on);

		void setUnicodeMode(bool on);

		void setNoCaptureGroupMode(bool on);

	private:
		uint32_t* m_regData;
		ConstString m_regStr;
		bool m_isIgnoreCase : 1;
		bool m_isGlobalSearchMode : 1;
		bool m_isDotMatchNewLine : 1;
		bool m_isMultiLineMode : 1;
		bool m_isLooseMode : 1;
		bool m_isUnicodeMode : 1;
		bool m_isNoCaptureMode : 1;
		bool m_isRegDataManaged : 1;
	};
}

/*
	更方便的正则表达式书写方式，无需考虑对 \ 进行转义。
	但是，由于宏展开的原因，需要注意不成对的括号的引号，使用时，需要用
	转义字符。
	例如：

	const wchar_t* reg = _XR(\w+\d*,([a-z]{0,5}));
	const wchar_t* reg = XIYUE_REG_STRING(\w+\d*,([a-z]{0,5}));

	实际上等价于：

	const wchar_t* reg = LR"(\w+\d*,([a-z]{0,5}))";
*/
#define __XIYUE_REGEX_LR_STRING(exp) LR##exp
#define __XIYUE_REGEX_STRING(exp) __XIYUE_REGEX_LR_STRING(#exp)
#define __XIYUE_REGEX_ADDPAREN(exp) __XIYUE_REGEX_STRING((exp))
#define XIYUE_REG_STRING(...) __XIYUE_REGEX_ADDPAREN(__VA_ARGS__)
#define _XR(...) XIYUE_REG_STRING(__VA_ARGS__)

#define RE_LP \x28		// (
#define RE_RP \x29		// )
#define RE_DQ \x22		// "
#define RE_SQ \x27		// '
