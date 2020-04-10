#include "stdafx.h"
#include <time.h>
#include "xiyue_console_log_consumer.h"

using namespace std;
using namespace xiyue;

#ifdef _WIN32
void printError(const wchar_t* msg)
{
	HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO screenBubberInfo;
	ZeroMemory(&screenBubberInfo, sizeof(screenBubberInfo));
	GetConsoleScreenBufferInfo(hStdErr, &screenBubberInfo);
	WORD attr = screenBubberInfo.wAttributes;
	SetConsoleTextAttribute(hStdErr, FOREGROUND_INTENSITY | FOREGROUND_RED | (attr & 0xf0));
	fwprintf(stderr, L"%s\n", msg);
	SetConsoleTextAttribute(hStdErr, attr);
}

void printWarning(const wchar_t* msg)
{
	HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO screenBubberInfo;
	ZeroMemory(&screenBubberInfo, sizeof(screenBubberInfo));
	GetConsoleScreenBufferInfo(hStdErr, &screenBubberInfo);
	WORD attr = screenBubberInfo.wAttributes;
	SetConsoleTextAttribute(hStdErr, FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED | (attr & 0xf0));
	fwprintf(stderr, L"%s\n", msg);
	SetConsoleTextAttribute(hStdErr, attr);
}

void printInfo(const wchar_t* msg)
{
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO screenBubberInfo;
	ZeroMemory(&screenBubberInfo, sizeof(screenBubberInfo));
	GetConsoleScreenBufferInfo(hStdOut, &screenBubberInfo);
	WORD attr = screenBubberInfo.wAttributes;
	SetConsoleTextAttribute(hStdOut, FOREGROUND_INTENSITY | FOREGROUND_GREEN | (attr & 0xf0));
	wprintf(L"%s\n", msg);
	SetConsoleTextAttribute(hStdOut, attr);
}

void printNormal(const wchar_t* msg)
{
	wprintf(L"%s\n", msg);
}
#elif __linux__
void printError(const wchar_t* msg)
{
	fwprintf(stderr, L"%s\n", msg);
}

void printWarning(const wchar_t* msg)
{
	fwprintf(stderr, L"%s\n", msg);
}

void printInfo(const wchar_t* msg)
{
	wprintf(L"%s\n", msg);
}

void printNormal(const wchar_t* msg)
{
	wprintf(L"%s\n", msg);
}
#endif

const wchar_t* LogLevel_toString(LogLevel level)
{
	switch (level)
	{
	case LogLevel_fatal:
		return L"fatal";
	case LogLevel_error:
		return L"error";
	case LogLevel_warning:
		return L"warning";
	case LogLevel_essential:
		return L"essential";
	case LogLevel_info:
		return L"info";
	case LogLevel_debug:
		return L"debug";
	default:
		return L"unknown";
	}
}

void ConsoleLogConsumer::comsumeMessage(const LogContent* content)
{
	wstring msg;
	wchar_t buffer[64];
	tm timeInfo = { 0 };
#ifdef _WIN32
	localtime_s(&timeInfo, &content->timestamp);
#elif __linux__
	localtime_r(&content->timestamp, &timeInfo);
#endif

	if (timeInfo.tm_hour < 10)
		msg.append(L"0");
	msg.append(itow(buffer, timeInfo.tm_hour));
	msg.append(L":");
	if (timeInfo.tm_min < 10)
		msg.append(L"0");
	msg.append(itow(buffer, timeInfo.tm_min));
	msg.append(L":");
	if (timeInfo.tm_sec < 10)
		msg.append(L"0");
	msg.append(itow(buffer, timeInfo.tm_sec));
	msg.append(L" - ");

	msg.append(L"[").append(content->loggerName).append(L"] ");
	msg.append(LogLevel_toString(content->level)).append(L" : ");

	msg.append(content->message);

	switch (content->level)
	{
	case LogLevel_fatal:
	case LogLevel_error:
		printError(msg.c_str());
		break;
	case LogLevel_warning:
	case LogLevel_essential:
		printWarning(msg.c_str());
		break;
	case LogLevel_info:
		printInfo(msg.c_str());
		break;
	case LogLevel_debug:
		printNormal(msg.c_str());
		break;
	}
}
