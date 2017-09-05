#pragma once
#include "AstNodes.h"
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace Carbon
{
	class ThreadPool;

	class ParallelTaskGroup
	{
		friend class ThreadPool;
	private:
		std::queue<std::function<void()>> Children;
		std::mutex mutex;
		std::condition_variable signalTaskDone;
		int workingThreads = 0;
		bool done = false;
		int MaxDegreeOfParallelism = 0;
		void AddTask(std::function<void()> task);
	public:
		bool IsDone() const;
		void WaitUntilDone();
	};

	class ThreadPool
	{
	private:
		std::deque<ParallelTaskGroup> tasks;
		std::vector<std::thread> threads;
		bool beingDisposed;
		void ThreadMethod();
		int numberOfThreads;
		std::mutex mutex;
		int workingThreads = 0;
		bool workers_enabled = true;
		std::condition_variable signalWorkerDone;
		std::condition_variable signalMasterDone;
		void InitializeThreads();
	public:
		void SubmitForExecution(std::function<void()> task);
		template<typename TLambdaContainer> void SubmitForExecution(TLambdaContainer tasks, int maxDegreeOfParallelism = 0);
		ThreadPool();
		ThreadPool(int numberOfThreads);
		~ThreadPool();
	};

	template <typename TLambdaContainer>
	void ThreadPool::SubmitForExecution(TLambdaContainer tasks, int maxDegreeOfParallelism)
	{
		if (maxDegreeOfParallelism <= 0)
		{
			maxDegreeOfParallelism = this->numberOfThreads;
		}
		std::unique_lock<std::mutex> lock(this->mutex); // aquire lock with lock guard 
		this->workers_enabled = false; // block worker threads
		// need to wait until readers are done
		this->signalWorkerDone.wait(lock, [&] { return this->workingThreads == 0; });
		// add tasks to the structure
		this->tasks.emplace_back();
		auto& group = this->tasks.back();
		group.MaxDegreeOfParallelism = maxDegreeOfParallelism;
		for (auto& task:tasks)
		{
			group.AddTask(task);
		}
		// done
		this->workers_enabled = true; // enable workers again
		lock.unlock(); // unlock 
		// signal worker threads to continue
		signalMasterDone.notify_all();
	}
}
