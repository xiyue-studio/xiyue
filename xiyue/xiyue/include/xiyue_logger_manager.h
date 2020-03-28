#pragma once
#include "xiyue_logger.h"

namespace xiyue
{
	//////////////////////////////////////////////////////////////////////////
	// 比较简洁的调用方式，用法： XIYUE_LOG_ERROR("File '%s' is not exist.", fileName);
#define XIYUE_LOG(level, msg, ...) ::xiyue::LoggerManager::getInstance()->getLogger(XIYUE_STRING(__FUNCTION__)) \
	->level(::xiyue::formatString(XIYUE_STRING(msg), __VA_ARGS__))

#define XIYUE_LOG_FATAL(msg, ...) XIYUE_LOG(fatal, msg, __VA_ARGS__)
#define XIYUE_LOG_ERROR(msg, ...) XIYUE_LOG(error, msg, __VA_ARGS__)
#define XIYUE_LOG_WARNING(msg, ...) XIYUE_LOG(warning, msg, __VA_ARGS__)
#define XIYUE_LOG_ESSENTIAL(msg, ...) XIYUE_LOG(essential, msg, __VA_ARGS__)
#define XIYUE_LOG_INFO(msg, ...) XIYUE_LOG(info, msg, __VA_ARGS__)
#define XIYUE_LOG_DEBUG(msg, ...) XIYUE_LOG(debug, msg, __VA_ARGS__)

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
		Logger* getLogger(const wchar_t* loggerName);
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
		std::unordered_map<std::wstring, Logger*> m_loggerMap;
		std::list<LogConsumer*> m_logConsumers;

		static LoggerManager m_singleInstance;
	};
}
