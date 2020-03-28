#include "pch.h"

using namespace std;
using namespace xiyue;

TEST(LogSystemTest, basicTest)
{
	class BasicTestConsumer : public LogConsumer
	{
	public:
		virtual void comsumeMessage(const LogContent* content) override {
			EXPECT_STREQ(content->message, m_realMessages[m_msgNum]);
			m_msgNum++;
		}

		void setStage(int stage) {
			switch (stage)
			{
			case 1:
				EXPECT_EQ(m_msgNum, 4);
				break;
			case 2:
				EXPECT_EQ(m_msgNum, 6);
				break;
			case 3:
				EXPECT_EQ(m_msgNum, 0);
				break;
			default:
				EXPECT_TRUE(!"Should not reach here.");
				break;
			}

			m_stage = stage;
			m_msgNum = 0;
		}

	private:
		int m_stage = 0;
		int m_msgNum = 0;
		const wchar_t* m_realMessages[6] = {
			L"Fatal error occured!",
			L"Error message is I am an error..",
			L"Warning occured 1234 times.",
			L"Essentail message here.",
			L"默认情况下不会显示 info 级别的 log。",
			L"Debug infomation."
		};
	} testConsumer;

	LoggerManager::getInstance()->addLogConsumer(&testConsumer);

	XIYUE_LOG_FATAL("Fatal error occured!");
	XIYUE_LOG_ERROR("Error message is %s.", L"I am an error.");
	XIYUE_LOG_WARNING("Warning occured %d times.", 1234);
	XIYUE_LOG_ESSENTIAL("Essentail message here.");
	XIYUE_LOG_INFO("默认情况下不会显示 info 级别的 log。");
	XIYUE_LOG_DEBUG("Debug infomation.");

	XIYUE_SET_LOG_LEVEL(LogLevel_all);
	testConsumer.setStage(1);

	XIYUE_LOG_FATAL("Fatal error occured!");
	XIYUE_LOG_ERROR("Error message is %s.", L"I am an error.");
	XIYUE_LOG_WARNING("Warning occured %d times.", 1234);
	XIYUE_LOG_ESSENTIAL("Essentail message here.");
	XIYUE_LOG_INFO("默认情况下不会显示 info 级别的 log。");
	XIYUE_LOG_DEBUG("Debug infomation.");

	XIYUE_SET_LOG_LEVEL(LogLevel_none);
	testConsumer.setStage(2);

	XIYUE_LOG_FATAL("Fatal error occured!");
	XIYUE_LOG_ERROR("Error message is %s.", L"I am an error.");
	XIYUE_LOG_WARNING("Warning occured %d times.", 1234);
	XIYUE_LOG_ESSENTIAL("Essentail message here.");
	XIYUE_LOG_INFO("默认情况下不会显示 info 级别的 log。");
	XIYUE_LOG_DEBUG("Debug infomation.");

	XIYUE_SET_LOG_LEVEL(LogLevel_default);
	testConsumer.setStage(3);
	LoggerManager::getInstance()->removeLogConsumer(&testConsumer);
}
