#include "stdafx.h"
#include "xiyue_logger_manager.h"
#include "xiyue_console_log_consumer.h"

using namespace std;
using namespace xiyue;

LoggerManager LoggerManager::m_singleInstance;
static ConsoleLogConsumer m_defaultConsumer;

LoggerManager::LoggerManager()
{
	addLogConsumer(&m_defaultConsumer);
}

LoggerManager* LoggerManager::getInstance()
{
	return &m_singleInstance;
}
#ifdef WIN32
Logger* LoggerManager::getLogger(const wchar_t* loggerName)
#else
Logger* LoggerManager::getLogger(const char* loggerName)
#endif
{
	auto it = m_loggerMap.find(loggerName);
	if (it == m_loggerMap.end())
	{
		Logger* logger = new Logger(loggerName, this);
		m_loggerMap.insert(make_pair(loggerName, logger));

		return logger;
	}

	return it->second;
}

void LoggerManager::addLogConsumer(LogConsumer* consumer)
{
	auto it = find(m_logConsumers.begin(), m_logConsumers.end(), consumer);
	if (it == m_logConsumers.end())
		m_logConsumers.push_back(consumer);
}

void LoggerManager::removeLogConsumer(LogConsumer* consumer)
{
	auto it = find(m_logConsumers.begin(), m_logConsumers.end(), consumer);
	if (it != m_logConsumers.end())
		m_logConsumers.erase(it);
}

void LoggerManager::report(const LogContent* content)
{
	if (content->level > m_maxLogLevel)
		return;

	for (auto consumer : m_logConsumers)
	{
		consumer->comsumeMessage(content);
	}
}
