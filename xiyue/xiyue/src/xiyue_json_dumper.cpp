#include "stdafx.h"
#include "xiyue_json_dumper.h"
#include "xiyue_string_format.h"
#include "xiyue_logger_manager.h"
#include "xiyue_string_file_writer.h"

using namespace std;
using namespace xiyue;

JsonDumper::JsonDumper()
{
	m_indentChar = ' ';
	m_indentSize = 4;
	m_isCompactMode = false;
	m_needSortKeys = false;
	m_newLineChars = L"\n";
}

uint32_t checkOutMembers(const JsonObject& obj, bool needSortKeys, vector<JsonMember>& memberBuffer, JsonMember* &members)
{
	auto memberValue = obj.data()->memberValue;
	uint32_t memberCount = memberValue.valueSize;
	if (needSortKeys)
	{
		memberBuffer.resize(memberValue.valueSize);
		copy(members, members + memberCount, memberBuffer.begin());
		sort(memberBuffer.begin(), memberBuffer.end(), [](const JsonMember& l, const JsonMember& r) {
			return l.key < r.key;
		});
		members = &memberBuffer[0];
	}
	else
	{
		members = memberValue.members;
	}

	return memberCount;
}

ConstString JsonDumper::dumpToString(const JsonObject& root)
{
	if (!root.isObject())
	{
		XIYUE_LOG_ERROR("Cannot dump non-object json element.");
		return L""_cs;
	}

	wstring result;
	if (m_isCompactMode)
		dumpObjectCompact(root, result);
	else
		dumpObject(root, result, 0);

	return ConstString(result);
}

void JsonDumper::dumpObjectCompact(const JsonObject& obj, wstring& buffer)
{
	buffer.push_back('{');
	// 收集成员
	JsonMember* members;
	uint32_t memberCount = checkOutMembers(obj, m_needSortKeys, m_memberBuffer, members);
	// 写入各个成员
	for (uint32_t i = 0; i < memberCount; ++i)
	{
		buffer.push_back('"');
		appendStringToBuffer(members[i].key.stringValue, members[i].key.strLen, buffer);
		buffer.append(L"\":");
		dumpJsonObjectCompact(*members[i].value, buffer);
		if (i + 1 != memberCount)
			buffer.push_back(',');
	}
	buffer.push_back('}');
}

void JsonDumper::dumpObject(const JsonObject& obj, wstring& buffer, int indentLevel)
{
	buffer.push_back('{');
	buffer.append(m_newLineChars);
	indentLevel += m_indentSize;
	// 收集成员
	JsonMember* members;
	uint32_t memberCount = checkOutMembers(obj, m_needSortKeys, m_memberBuffer, members);
	// 写入各个成员
	for (uint32_t i = 0; i < memberCount; ++i)
	{
		buffer.append(indentLevel, m_indentChar);
		buffer.push_back('"');
		appendStringToBuffer(members[i].key.stringValue, members[i].key.strLen, buffer);
		buffer.append(L"\": ");
		dumpJsonObject(*members[i].value, buffer, indentLevel);
		if (i + 1 != memberCount)
			buffer.push_back(',');
		buffer.append(m_newLineChars);
	}
	indentLevel -= m_indentSize;
	buffer.append(indentLevel, m_indentChar);
	buffer.push_back('}');
}

void JsonDumper::dumpListCompact(const JsonObject& obj, wstring& buffer)
{
	buffer.push_back('[');
	// 写入各个成员
	auto members = obj.data()->listValue.values;
	uint32_t memberCount = obj.data()->listValue.valueSize;
	for (uint32_t i = 0; i < memberCount; ++i)
	{
		dumpJsonObjectCompact(*members[i], buffer);
		if (i + 1 != memberCount)
			buffer.push_back(',');
	}
	buffer.push_back(']');
}

void JsonDumper::dumpList(const JsonObject& obj, wstring& buffer, int indentLevel)
{
	buffer.push_back('[');
	buffer.append(m_newLineChars);
	indentLevel += m_indentSize;
	// 写入各个成员
	auto members = obj.data()->listValue.values;
	uint32_t memberCount = obj.data()->listValue.valueSize;
	for (uint32_t i = 0; i < memberCount; ++i)
	{
		buffer.append(indentLevel, m_indentChar);
		dumpJsonObject(*members[i], buffer, indentLevel);
		if (i + 1 != memberCount)
			buffer.push_back(',');
		buffer.append(m_newLineChars);
	}
	indentLevel -= m_indentSize;
	buffer.append(indentLevel, m_indentChar);
	buffer.push_back(']');
}

void JsonDumper::dumpInt(const JsonObject& obj, wstring& buffer)
{
	wchar_t intStr[32];
	buffer.append(itow(intStr, obj.intValue()));
}

void JsonDumper::dumpReal(const JsonObject& obj, wstring& buffer)
{

	if (m_realNumberFormat.isEmpty())
	{
		wchar_t realStr[32];
		buffer.append(ftow(realStr, obj.realValue()));
	}
	else
	{
		wstring formatedNumber;
		xiyue_formatNumber(formatedNumber, m_realNumberFormat, obj.realValue());
		buffer.append(formatedNumber);
	}
}

void JsonDumper::dumpBoolean(const JsonObject& obj, wstring& buffer)
{
	if (obj.data()->boolValue)
		buffer.append(L"true");
	else
		buffer.append(L"false");
}

void JsonDumper::dumpNull(std::wstring& buffer)
{
	buffer.append(L"null");
}

void JsonDumper::appendStringToBuffer(const wchar_t* str, uint32_t len, wstring& buffer)
{
	const wchar_t* p = str;
	const wchar_t* p2 = p;
	const wchar_t* pEnd = str + len;

	while (p2 < pEnd)
	{
		if (*p2 == '"')
		{
			buffer.append(p, p2);
			buffer.append(L"\\\"");
			p = p2 + 1;
		}

		p2++;
	}

	buffer.append(p, p2);
}

void JsonDumper::dumpJsonObjectCompact(const JsonObject& obj, wstring& buffer)
{
	switch (obj.getType())
	{
	case Json_null:
		dumpNull(buffer);
		break;
	case Json_int:
		dumpInt(obj, buffer);
		break;
	case Json_real:
		dumpReal(obj, buffer);
		break;
	case Json_boolean:
		dumpBoolean(obj, buffer);
		break;
	case Json_string:
		dumpString(obj, buffer);
		break;
	case Json_list:
		dumpListCompact(obj, buffer);
		break;
	case Json_object:
		dumpObjectCompact(obj, buffer);
		break;
	}
}

void JsonDumper::dumpJsonObject(const JsonObject& obj, wstring& buffer, int indentLevel)
{
	switch (obj.getType())
	{
	case Json_null:
		dumpNull(buffer);
		break;
	case Json_int:
		dumpInt(obj, buffer);
		break;
	case Json_real:
		dumpReal(obj, buffer);
		break;
	case Json_boolean:
		dumpBoolean(obj, buffer);
		break;
	case Json_string:
		dumpString(obj, buffer);
		break;
	case Json_list:
		dumpList(obj, buffer, indentLevel);
		break;
	case Json_object:
		dumpObject(obj, buffer, indentLevel);
		break;
	}
}

void JsonDumper::dumpString(const JsonObject& obj, wstring& buffer)
{
	buffer.push_back('"');
	appendStringToBuffer(obj.data()->stringValue.stringValue, obj.data()->stringValue.strLen, buffer);
	buffer.push_back('"');
}

bool JsonDumper::dumpToFile(const ConstString& fileName, const JsonObject& root, StringEncoding encoding /*= StringEncoding_utf8*/)
{
	ConstString result = dumpToString(root);
	if (result.isEmpty())
		return false;

	StringFileWriter writer(result.length());
	if (!writer.open(fileName, encoding))
		return false;

	writer.writeString(result);
	return true;
}

void JsonDumper::setNewLineStyle(NewLineStyle newLineStyle)
{
	switch (newLineStyle)
	{
	case NewLineStyle_crlf:
		m_newLineChars = L"\r\n";
		break;
	case NewLineStyle_cr:
		m_newLineChars = L"\r";
		break;
	case NewLineStyle_lf:
	default:
		m_newLineChars = L"\n";
		break;
	}
}
