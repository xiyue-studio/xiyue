#pragma once
#include "xiyue_xy_re_program.h"
#include "xiyue_xy_re_thread.h"

namespace xiyue
{
	class XyReThreadPool
	{
	public:
		explicit XyReThreadPool(const XyReProgram* program);
		~XyReThreadPool();

	public:
		XyReThread* allocThread();
		XyReThread* splitThread(const XyReThread* thread);
		void recycleThread(XyReThread* thread);

	private:
		std::vector<XyReThread*> m_allocatedThreads;
		std::vector<XyReThread*> m_freeThreads;
		size_t m_threadSize;
	};
}
