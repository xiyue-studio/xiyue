#include "stdafx.h"
#include "xiyue_xy_re_thread_pool.h"

using namespace std;
using namespace xiyue;

static void constructThread(XyReThread* t, size_t tSize)
{
	memset(t, 0, sizeof(XyReThread));
	memset(t + 1, 0xff, tSize - sizeof(XyReThread));
}

XyReThreadPool::XyReThreadPool(const XyReProgram* program)
{
	m_threadSize = sizeof(XyReThread) + sizeof(XyReGroupPosition) * (program->namedGroupCount + program->numberGroupCount);
}

XyReThreadPool::~XyReThreadPool()
{
	for (auto t : m_allocatedThreads)
		free(t);
}

XyReThread* XyReThreadPool::allocThread()
{
	XyReThread* t;
	if (m_freeThreads.empty())
	{
		t = (XyReThread*)malloc(m_threadSize);
		m_allocatedThreads.push_back(t);
	}
	else
	{
		t = m_freeThreads.back();
		m_freeThreads.pop_back();
	}

	constructThread(t, m_threadSize);
	return t;
}

XyReThread* XyReThreadPool::splitThread(const XyReThread* thread)
{
	XyReThread* t;
	if (m_freeThreads.empty())
	{
		t = (XyReThread*)malloc(m_threadSize);
		m_allocatedThreads.push_back(t);
	}
	else
	{
		t = m_freeThreads.back();
		m_freeThreads.pop_back();
	}

	memcpy(t, thread, m_threadSize);
	return t;
}

void XyReThreadPool::recycleThread(XyReThread* thread)
{
	m_freeThreads.push_back(thread);
}
