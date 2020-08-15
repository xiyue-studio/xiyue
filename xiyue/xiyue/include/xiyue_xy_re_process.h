#pragma once
#include "xiyue_xy_re_program.h"
#include "xiyue_xy_re_thread_pool.h"

namespace xiyue
{
	/**
		����ƥ��Ļ�����ʩ����������Ҫ���ݲ�ͬ����������ͬ���ࡣ
	*/
	class XyReProcess
	{
	public:
		explicit XyReProcess(const XyReProgram* program);
		virtual ~XyReProcess();

	public:
		/*
			ע�⣬Process �ڲ����ᱣ���ַ������� Process ��Ч�ڼ䣬
			��Ҫ�����߱�֤�ַ�������Ч�ԡ�

			isFragment = true ���Զ����ǰ���ͺ���ַ���
			�����������ַ����ᱻ����Ϊ 0��
		*/
		void setTargetString(const wchar_t* strBegin, const wchar_t* strEnd, bool isFragment = false);
		void setTargetString(const ConstString& str) { setTargetString(str.begin(), str.end()); }
		void setTargetString(const wchar_t* str) { setTargetString(str, str + wcslen(str)); }
		void setTargetString(const std::wstring& str) { setTargetString(&*str.begin(), &*str.end()); }

		/*
			���������Ŀ���ַ���ֻ��һ��Ƭ�Σ�Ϊ����ȷ���� ^, $, \b��
			��Ҫ�ṩǰ���ַ��ͺ���ַ���
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
			bool succeeded;					///< ���ӽ���Ƿ�ɹ�
			const wchar_t* stopPos;			///< �ɹ��Ļ������ӵ���һ�˵㣬��ǰ����㣬������յ�
			XyReThread* succeededThread;	///< �ɹ�ƥ��ʱ���̱߳��ݡ���ҪΪ�˻�ȡ���񵽵���
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

		// ���ӻ��棬�� PC �� SP �̶���ǰ���£�û��Ҫ�ظ�Ԥ�⻷�ӽ��
		LookAroundCacheMap m_lookBehindCaches;
		LookAroundCacheMap m_lookAheadCaches;
	};
}
