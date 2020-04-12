#pragma once

namespace xiyue
{
	class LoggerManager;

	enum LogLevel
	{
		LogLevel_none = 0,
		LogLevel_fatal,
		LogLevel_error,
		LogLevel_warning,
		LogLevel_essential,
		LogLevel_info,
		LogLevel_debug,
		LogLevel_all,
		LogLevel_default = LogLevel_essential
	};

	class Logger
	{
	public:
#ifdef WIN32
		Logger(const wchar_t* loggerName, LoggerManager* manager);
#else
		Logger(const char* loggerName, LoggerManager* manager);
#endif

	public:
		inline void fatal(const wchar_t* msg) { log(LogLevel_fatal, msg); }
		inline void error(const wchar_t* msg) { log(LogLevel_error, msg); }
		inline void warning(const wchar_t* msg) { log(LogLevel_warning, msg); }
		inline void essential(const wchar_t* msg) { log(LogLevel_essential, msg); }
		inline void info(const wchar_t* msg) { log(LogLevel_info, msg); }
		inline void debug(const wchar_t* msg) { log(LogLevel_debug, msg); }

	protected:
		void log(LogLevel level, const wchar_t* msg);

	protected:
#ifdef WIN32
		const std::wstring m_loggerName;
#else
		const std::string m_loggerName;
#endif
		LoggerManager* m_manager;
	};

	/*
		TODO 需要将这个函数修改成线程安全的。
	*/
	const wchar_t* formatString(const wchar_t* format, ...);
}
