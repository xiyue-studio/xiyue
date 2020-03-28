#pragma once
#include "xiyue_logger_manager.h"

namespace xiyue
{
	class ConsoleLogConsumer : public LogConsumer
	{
	public:
		virtual void comsumeMessage(const LogContent* content) override;
	};
}
