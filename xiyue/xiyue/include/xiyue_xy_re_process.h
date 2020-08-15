#pragma once
#include "xiyue_xy_re_program.h"
#include "xiyue_xy_re_thread_pool.h"

namespace xiyue
{
	/**
		正则匹配的基础设施，后续，需要根据不同场景派生不同的类。
	*/
	class XyReProcess
	{
	public:
		explicit XyReProcess(const XyReProgram* program);
		virtual ~XyReProcess();

	public:
		/*
			注意，Process 内部不会保持字符串，在 Process 有效期间，
			需要调用者保证字符串的有效性。

			isFragment = true 会自动填充前跟和后继字符，
			否则，这两个字符都会被设置为 0。
		*/
		void setTargetString(const wchar_t* strBegin, const wchar_t* strEnd, bool isFragment = false);
		void setTargetString(const ConstString& str) { setTargetString(str.begin(), str.end()); }
		void setTargetString(const wchar_t* str) { setTargetString(str, str + wcslen(str)); }
		void setTargetString(const std::wstring& str) { setTargetString(&*str.begin(), &*str.end()); }

		/*
			如果给定的目标字符串只是一个片段，为了正确处理 ^, $, \b，
			需要提供前跟字符和后继字符。
		*/
		void setFormerChar(wchar_t ch) { m_formerChar = ch; }
		void setLatterChar(wchar_t ch) { m_latterChar = ch; }

		void setIgnoreCase(bool on) { m_isIgnoreCase = on; }
		void setMultiLineMode(bool on) { m_isMultiLineMode = on; }
		void setDotMatchNewLine(bool on) { m_isDotMatchNewLine = on; }
		void setUnicodeMode(bool on) { m_isUnicodeMode = on; }
		void setMatchWholeString(bool on) { m_isMatchWholeString = on; }
		void setJustTestMatch(bool on) { m_isJustTestMatch = on; }

	public:
		void match();

		void start();
		void start(const wchar_t* sp);
		void start(uint32_t index);
		void startAt(XyReProgramPC pc, const wchar_t* sp);
		void startAt(XyReProgramPC pc, const wchar_t* sp, const wchar_t* ep);
		void stepThread(XyReThread* t);

	public:
		void reset();
		XyReProcess* clone();
		bool isSucceeded() { return m_isSucceeded; }
		bool isNoNeedTestMore() { return m_isNoNeedTestMore; }
		const wchar_t* matchedStart() { return m_strBegin + m_startIndex; }
		uint32_t matchedLength() { return m_succThread.matchedLength; }
		uint32_t startMatchIndex() { return m_startIndex; }
		const XyReProgram* getProgram() const { return m_program; }

	public:
		struct MatchedThreadState
		{
			uint32_t matchedLength;
			XyReThread* thread;
		};

		typedef std::pair<XyReProgramPC, const wchar_t*> LookAroundCacheKey;

		struct LookAroundCache
		{
			bool succeeded;					///< 环视结果是否成功
			const wchar_t* stopPos;			///< 成功的话，环视的另一端点，向前是起点，向后是终点
			XyReThread* succeededThread;	///< 成功匹配时的线程备份。主要为了获取捕获到的组
		};

		typedef std::map<LookAroundCacheKey, LookAroundCache> LookAroundCacheMap;

	public:
		void startAtPc(XyReProgramPC pc);

		void removeThread(XyReThread* t);
		void runThreads();
		const wchar_t* stringBegin() const { return m_strBegin; }
		const wchar_t* stringEnd() const { return m_strEnd; }
		const wchar_t* stringPosition() const { return m_sp; }
		const wchar_t* lastSearchStoppedPosition() const { return m_lastSearchStoppedPos; }
		wchar_t formerChar() const { return m_formerChar; }
		wchar_t latterChar() const { return m_latterChar; }

		bool isIgnoreCase() const { return m_isIgnoreCase; }
		bool isMultiLineMode() const { return m_isMultiLineMode; }
		bool isDotMatchNewLine() const { return m_isDotMatchNewLine; }
		bool isUnicodeMode() const { return m_isUnicodeMode; }
		bool isMatchWholeString() const { return m_isMatchWholeString; }

		LookAroundCache* findLookBehindCache(XyReProgramPC pc, const wchar_t* sp);
		LookAroundCache* findLookAheadCache(XyReProgramPC pc, const wchar_t* sp);

		LookAroundCacheMap& lookBehindCache() { return m_lookBehindCaches; }
		LookAroundCacheMap& lookAheadCache() { return m_lookAheadCaches; }
		XyReThreadPool* threadPool() { return m_threadPool; }

		XyReThread* matchedThread() { return m_succThread.thread; }

		void checkMatchState();

		static const int CHAR_SIZE = 2;

	private:
		const XyReProgram* m_program;
		const wchar_t* m_strBegin;
		const wchar_t* m_strEnd;
		const wchar_t* m_sp;
		XyReThreadPool* m_threadPool;

		bool m_isIgnoreCase : 1;
		bool m_isMultiLineMode : 1;
		bool m_isDotMatchNewLine : 1;
		bool m_isUnicodeMode : 1;
		bool m_isSucceeded : 1;
		bool m_isFailed : 1;
		bool m_isMatchWholeString : 1;
		bool m_isJustTestMatch : 1;
		bool m_isNoNeedTestMore : 1;
		bool m_isThreadPoolManaged : 1;

		wchar_t m_formerChar;
		wchar_t m_latterChar;
		uint32_t m_startIndex;

		const wchar_t* m_lastSearchStoppedPos;

		std::list<XyReThread*> m_workingThreads;
		std::list<XyReThread*> m_suspendThreads;
		std::list<XyReThread*> m_delayedThreads;
		std::map<uint32_t, MatchedThreadState> m_abandonWinners;
		std::set<XyReThread*> m_abandonThreads;
		MatchedThreadState m_succThread;

		// 环视缓存，在 PC 和 SP 固定的前提下，没必要重复预测环视结果
		LookAroundCacheMap m_lookBehindCaches;
		LookAroundCacheMap m_lookAheadCaches;
	};
}
