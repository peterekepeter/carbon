#pragma once
#include "AstNodes.h"
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <string>

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
		void ThreadMethod(int id);
		int numberOfThreads;
		std::mutex mutex;
		int workingThreads = 0;
		bool workers_enabled = false;
		std::condition_variable signalWorkerDone;
		std::condition_variable signalMasterDone;
		void InitializeThreads();
	public:
		ParallelTaskGroup* SubmitForExecution(std::function<void()> task);
		template<typename TLambdaContainer> ParallelTaskGroup* SubmitForExecution(TLambdaContainer tasks, int maxDegreeOfParallelism = 0);
		ThreadPool();
		ThreadPool(int numberOfThreads);
		~ThreadPool();
	};

	template <typename TLambdaContainer>
	ParallelTaskGroup* ThreadPool::SubmitForExecution(TLambdaContainer tasks, int maxDegreeOfParallelism)
	{
		if (maxDegreeOfParallelism <= 0)
		{
			maxDegreeOfParallelism = this->numberOfThreads;
		}
		//std::cerr << "master lock pool\n";
		std::unique_lock<std::mutex> lock(this->mutex); // aquire lock with lock guard 
		this->workers_enabled = false; // block worker threads
		// need to wait until readers are done
		//std::cerr << "master signalWorkerDone.wait\n";
		this->signalWorkerDone.wait(lock, [&] { return this->workingThreads == 0; });
		// remove finished groups
		while(!this->tasks.empty() && this->tasks.front().IsDone())
		{
			this->tasks.pop_front();
		}
		// add tasks to the structure
		this->tasks.emplace_back();
		auto& group = this->tasks.back();
		group.MaxDegreeOfParallelism = maxDegreeOfParallelism;
		int i = 0;
		for (auto& task:tasks)
		{
			group.AddTask(task);
			i++;
		}
		// done
		//std::cerr << "master thread posted " + std::to_string(i) + " tasks in 1 group\n";
		this->workers_enabled = true; // enable workers again
		//std::cerr << "master unlock pool\n";
		lock.unlock(); // unlock 
		// signal worker threads to continue
		//std::cerr << "master notify workers\n";
		signalMasterDone.notify_all();
		return &group;
	}
}
