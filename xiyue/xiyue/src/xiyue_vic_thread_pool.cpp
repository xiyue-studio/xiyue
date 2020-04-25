#include "stdafx.h"
#include "xiyue_vic_thread_pool.h"

using namespace xiyue;

void* VicThreadPool::threadRoutine(void* args)
{
	VicThreadPool* pool = (VicThreadPool*)args;
	while (pool->m_running)
	{
		VicThreadTask t;
		// critical area begin
		{
			std::unique_lock<std::mutex> lock(pool->m_mutex);
			if (pool->m_tasks.size() > 0)
			{
				t = pool->m_tasks.front();
				pool->m_tasks.pop_front();
			}
			else {
				// no task, hang up
				pool->m_cond.wait(lock);
			}
		}
		// critical area end
		if (t.run != nullptr)
			t.run(t.args);
	}
}

VicThreadPool::VicThreadPool(int count)
	: m_running(true)
{
	m_threads.resize(0);
	for (int i = 0; i < count; i++)
	{
		std::thread* t = new std::thread(threadRoutine, this);
		m_threads.push_back(t);
	}
}

VicThreadPool::~VicThreadPool()
{
	m_running = false;
	// wake up all pending thread.
	m_cond.notify_all();
	for (auto iter : m_threads)
	{
		iter->join();
		delete iter;
	}
}

void VicThreadPool::addTask(TaskFunc run, void* args)
{
	VicThreadTask t(run, args);
	{
		// critical area begin
		std::unique_lock<std::mutex> lock(m_mutex);
		m_tasks.push_back(t);
		// critical area end
	}
	m_cond.notify_one();
}
