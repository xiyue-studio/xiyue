#pragma once
#include "xiyue_json_object.h"
#include "xiyue_encoding.h"

namespace xiyue
{
	class JsonDumper
	{
	public:
		JsonDumper();
		virtual ~JsonDumper() = default;

	public:
		void setIndentSize(int indentSize, wchar_t indentChar = ' ') {
			m_indentSize = indentSize;
			m_indentChar = indentChar;
		}
		void setCompactMode(bool enableCompactMode) {
			m_isCompactMode = enableCompactMode;
		}
		void setRealNumberFormat(const ConstString& format) {
			m_realNumberFormat = format;
		}
		void setSortKeys(bool enableKeySort) {
			m_needSortKeys = enableKeySort;
		}
		void setNewLineStyle(NewLineStyle newLineStyle);

	public:
		ConstString dumpToString(const JsonObject& root);
		bool dumpToFile(const ConstString& fileName, const JsonObject& root, StringEncoding encoding = StringEncoding_utf8);

	protected:
		void dumpJsonObjectCompact(const JsonObject& obj, std::wstring& buffer);
		void dumpJsonObject(const JsonObject& obj, std::wstring& buffer, int indentLevel);
		void dumpObjectCompact(const JsonObject& obj, std::wstring& buffer);
		void dumpObject(const JsonObject& obj, std::wstring& buffer, int indentLevel);
		void dumpListCompact(const JsonObject& obj, std::wstring& buffer);
		void dumpList(const JsonObject& obj, std::wstring& buffer, int indentLevel);
		void dumpInt(const JsonObject& obj, std::wstring& buffer);
		void dumpReal(const JsonObject& obj, std::wstring& buffer);
		void dumpBoolean(const JsonObject& obj, std::wstring& buffer);
		void dumpNull(std::wstring& buffer);
		void dumpString(const JsonObject& obj, std::wstring& buffer);
		void appendStringToBuffer(const wchar_t* str, uint32_t len, std::wstring& buffer);

	protected:
		wchar_t m_indentChar;
		int m_indentSize;
		bool m_isCompactMode;
		ConstString m_realNumberFormat;
		bool m_needSortKeys;
		const wchar_t* m_newLineChars;
		std::vector<JsonMember> m_memberBuffer;
	};
}
