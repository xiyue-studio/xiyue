#include "stdafx.h"
#include "xiyue_json_document.h"
#include "xiyue_json_parser.h"
#include "xiyue_json_exception.h"
#include "xiyue_string_file_reader.h"
#include "xiyue_logger_manager.h"

using namespace std;
using namespace xiyue;

JsonDocument::JsonDocument()
{
	m_isRetainMode = false;
	m_ignoreRedundantComma = true;
	m_ignoreTopBrace = true;
}

bool JsonDocument::parse(const ConstString& jsonStr)
{
	JsonParser parser;
	m_rootObject = JsonObject();
	parser.setStringNeedCreate(!m_isRetainMode);
	parser.setTopBraceIgnored(m_ignoreTopBrace);
	parser.setRedundantCommaIgnored(m_ignoreRedundantComma);

	try
	{
		m_rootObject = parser.parseJson(jsonStr);
	}
	catch (JsonException& e)
	{
		e.logErrorMessage(jsonStr);
		return false;
	}

	return true;
}

bool JsonDocument::parseJsonFile(ConstString fileName, StringEncoding encoding /*= StringEncoding_utf8*/)
{
	StringFileReader reader;
	if (!reader.readFile(fileName, encoding))
	{
		XIYUE_LOG_ERROR("Read file `%s` failed.", fileName.cstr());
		return false;
	}

	return parse(reader.getText());
}
