#pragma once
#include "xiyue_json_errors.h"

namespace xiyue
{
	class JsonException
	{
	public:
		JsonException(JsonError errNum, uint32_t offset, uint32_t length);

	public:
		void logErrorMessage(const ConstString& srcText);

	protected:
		JsonError m_errorNum;
		uint32_t m_offset;
		uint32_t m_length;
	};
}
