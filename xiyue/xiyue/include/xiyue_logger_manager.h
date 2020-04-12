#pragma once
#include "xiyue_logger.h"

namespace xiyue
{
	//////////////////////////////////////////////////////////////////////////
	// 比较简洁的调用方式，用法： XIYUE_LOG_ERROR("File '%s' is not exist.", fileName);
#ifdef WIN32
#define XIYUE_LOG(level, msg, ...) ::xiyue::LoggerManager::getInstance()->getLogger(XIYUE_STRING(__FUNCTION__)) \
	->level(::xiyue::formatString(XIYUE_STRING(msg), __VA_ARGS__))
#else
#define XIYUE_LOG(level, ...) ::xiyue::LoggerManager::getInstance()->getLogger(__FUNCTION__) \
	->level(::xiyue::formatString(__VA_ARGS__))
#endif

#define XIYUE_LOG_FATAL(...) XIYUE_LOG(fatal, __VA_ARGS__)
#define XIYUE_LOG_ERROR(...) XIYUE_LOG(error, __VA_ARGS__)
#define XIYUE_LOG_WARNING(...) XIYUE_LOG(warning, __VA_ARGS__)
#define XIYUE_LOG_ESSENTIAL(...) XIYUE_LOG(essential, __VA_ARGS__)
#define XIYUE_LOG_INFO(...) XIYUE_LOG(info, __VA_ARGS__)
#define XIYUE_LOG_DEBUG(...) XIYUE_LOG(debug, __VA_ARGS__)

#define XIYUE_SET_LOG_LEVEL(logLevel) ::xiyue::LoggerManager::getInstance()->setMaxLogLevel(logLevel)

	struct LogContent
	{
		const wchar_t* loggerName;
		LogLevel level;
		time_t timestamp;
		const wchar_t* message;
	};

	class LogConsumer
	{
	public:
		virtual void comsumeMessage(const LogContent* content) = 0;
	};

	class LoggerManager
	{
	public:
		static LoggerManager* getInstance();

	public:
#ifdef WIN32
		Logger* getLogger(const wchar_t* loggerName);
#else
		Logger* getLogger(const char* loggerName);
#endif
		void addLogConsumer(LogConsumer* consumer);
		void removeLogConsumer(LogConsumer* consumer);
		void clearConsumers() { m_logConsumers.clear(); }

		void report(const LogContent* content);

		void setMaxLogLevel(LogLevel level) { m_maxLogLevel = level; }

	protected:
		LoggerManager();
		virtual ~LoggerManager() {}

	private:
		LogLevel m_maxLogLevel = LogLevel_essential;
#ifdef WIN32
		std::unordered_map<std::wstring, Logger*> m_loggerMap;
#else
		std::unordered_map<std::string, Logger*> m_loggerMap;
#endif
		std::list<LogConsumer*> m_logConsumers;

		static LoggerManager m_singleInstance;
	};
}
