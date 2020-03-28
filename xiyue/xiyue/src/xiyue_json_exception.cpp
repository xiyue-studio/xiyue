#include "stdafx.h"
#include "xiyue_json_exception.h"
#include "xiyue_logger_manager.h"

using namespace std;
using namespace xiyue;

JsonException::JsonException(JsonError errNum, uint32_t offset, uint32_t length)
	: m_errorNum(errNum)
	, m_offset(offset)
	, m_length(length)
{
}

void JsonException::logErrorMessage(const ConstString& srcText)
{
	ConstString token = srcText.substr(m_offset, m_length);
	XIYUE_LOG_ERROR("[xiyue.json] syntax error E%d near `%s` (offset: %u): %s.", m_errorNum, token.cstr(), m_offset,
		JsonError_toString(m_errorNum, token).cstr());
}
