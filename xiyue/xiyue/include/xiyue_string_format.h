#pragma once

namespace xiyue
{
	/*
		֧�ֵĸ�ʽ��

		���� 0.00 ���������֣�
		0 ��ʾ����ط�������֣������ 0 Ҳ��� 0��
		# ��ʾ����ط�������֣������û��Ҫ�� 0 �������

		ע�⣬����������� output��
	*/
	bool xiyue_formatNumber(std::wstring& output, const wchar_t* formatStr, double value);

	class ParseUtils
	{
	public:
		// ƥ�䵥���Ż���˫���Ű������ַ��������Ա�ת�������
		static bool matchString(ConstStringPointer& p, bool* hasEscapeChar, bool* hasUnmatchedQuote);

		static bool matchID(ConstStringPointer& p);

		// ƥ���������߸�������֧�� 1.23e-5 ������ʽ�Ŀ�ѧ�����������Դ�Сд
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
		�������� name_of_variable ��������ʽת���� nameOfVariable ����ʽ��
		�������֮ǰ����ж���ָ������� `name      of----case`�����ᱻ�ϲ���
		��ʼλ�õķ���ĸ�ַ��ᱻ���ԡ�
	*/
	ConstString xiyue_makeCamelCaseName(const ConstString& src, bool uppercaseFirstLetter);
}
