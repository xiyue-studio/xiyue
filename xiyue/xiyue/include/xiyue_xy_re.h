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

		/**
			�滻��ʽ�﷨��

			$number �� ${number} ��ʾ���������ã���� number �Ǹ���Ч�飬��ֱ���滻�����֡�
			${name} ��ʾ����������������á�name ��Чʱ����ֱ���滻 name �ַ�����
			$$ �滻Ϊ $��
			$& �滻Ϊ����ƥ����ȼ��� $0��
			$` �滻Ϊ prefix��
			$' �滻Ϊ suffix��
			$+ �滻Ϊ���һ�����ֲ����顣
			$_ �滻Ϊ�������봮��
			$U ������дת��ģʽ�������滻�Ĳ����鶼������ת���ɴ�д�ַ������滻��
			$E �ָ�Ĭ���滻ģʽ������ԭ�ַ������д�Сдת����
			$L ����Сдת��ģʽ�������滻�Ĳ����鶼������ת����Сд�ַ������滻��

			�����κ�δʶ���ָ�����ԭ����������� $ ���ţ���
		*/
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

		/**
			@sa XyReMatch::format()
		*/
		ConstString replace(const ConstString& srcStr, const ConstString& replacePattern);

		void compile();

		// ���� XyRe ������ʽ�������ʽָ��
		static uint32_t* buildProgram(const wchar_t* re);

	public:
		/*
			����ƥ���Ƿ���Դ�Сд��
		*/
		void setIgnoreCaseMode(bool on) { m_isIgnoreCase = on; }
		/*
			����ƥ���Ƿ���ȫ��ƥ�䡣

			ȫ��ƥ���� search �� replace ��ʱ�򣬻���������ƥ��Ľ����
			������ָ�ҵ�һ��ƥ��Ľ����
		*/
		void setGlobalSearchMode(bool on) { m_isGlobalSearchMode = on; }
		/*
			���� . Ԫ�ַ��Ƿ�Ҳ����ƥ�任�з���
		*/
		void setDotMatchNewLine(bool on) { m_isDotMatchNewLine = on; }
		/*
			���� ^ �� $ ��ƥ���ַ����Ŀ�ͷ�Ľ�β������ƥ���еĿ�ͷ�ͽ�β��

			on ��ʾ����ƥ����ͷ����β��
		*/
		void setMultiLineMode(bool on) { m_isMultiLineMode = on; }
		/*
			�����Ƿ�֧����ɢģʽ����ɢģʽ�£�����������ӿո񣬲����Զ�
			������ʽ����ע�͡�
		*/
		void setLooseMode(bool on) { m_isLooseMode = on; }

		void setUnicodeMode(bool on) { m_isUnicodeMode = on; }

		void setNoCaptureGroupMode(bool on) { m_isNoCaptureMode = on; }

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
	�������������ʽ��д��ʽ�����迼�Ƕ� \ ����ת�塣
	���ǣ����ں�չ����ԭ����Ҫע�ⲻ�ɶԵ����ŵ����ţ�ʹ��ʱ����Ҫ��
	ת���ַ���
	���磺

	const wchar_t* reg = _XR(\w+\d*,([a-z]{0,5}));
	const wchar_t* reg = XIYUE_REG_STRING(\w+\d*,([a-z]{0,5}));

	ʵ���ϵȼ��ڣ�

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
