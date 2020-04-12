#include "stdafx.h"
#include <time.h>
#include "xiyue_logger.h"
#include "xiyue_logger_manager.h"

using namespace std;
using namespace xiyue;

const wchar_t* xiyue::formatString(const wchar_t* format, ...)
{
	static thread_local wstring formattedText;

	if (format == nullptr)
		return L"";

	va_list args = nullptr;
	va_start(args, format);
	size_t len = _vscwprintf(format, args) + 1;
	wchar_t* buffer = (wchar_t*)malloc(sizeof(wchar_t) * len);
	memset(buffer, 0, sizeof(wchar_t) * len);
	_vsnwprintf_s(buffer, len, len, format, args);
	va_end(args);
	formattedText.assign(buffer);
	free(buffer);

	return formattedText.c_str();
}

Logger::Logger(const wchar_t* loggerName, LoggerManager* manager)
	: m_loggerName(loggerName)
	, m_manager(manager)
{
}

void Logger::log(LogLevel level, const wchar_t* msg)
{
	LogContent content;
	content.level = level;
	content.loggerName = m_loggerName.c_str();
	time(&content.timestamp);
	content.message = msg;

	m_manager->report(&content);
}
