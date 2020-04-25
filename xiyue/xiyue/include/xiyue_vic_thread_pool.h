#pragma once

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <list>
#include <thread>
#include <condition_variable>

namespace xiyue
{
	typedef void *(*TaskFunc)(void *);
	struct VicThreadTask
	{
		TaskFunc run;
		void* args;
		VicThreadTask()
		{
			this->run = nullptr;
			this->args = nullptr;
		}
		VicThreadTask(TaskFunc r, void* args)
		{
			this->run = r;
			this->args = args;
		}
	};

	class VicThreadPool
	{
	public:
		static void* threadRoutine(void* args);

	public:
		VicThreadPool(int count);
		~VicThreadPool();

		void addTask(const VicThreadTask* task);
		void addTask(TaskFunc run, void* args);

	private:
		std::list<std::thread*> m_threads;
		std::list<VicThreadTask> m_tasks;

		bool m_running;
		std::mutex m_mutex;
		std::condition_variable m_cond;
	};
}

#endif
