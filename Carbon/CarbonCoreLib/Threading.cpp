#include "Threading.h"
#include <array>

void Carbon::ParallelTaskGroup::AddTask(std::function<void()> task)
{
	this->mutex.lock();
	this->Children.push(task);
	this->mutex.unlock();
}

void Carbon::ThreadPool::ThreadMethod()
{
	std::deque<ParallelTaskGroup>::iterator taskGroupPtr;
	// check for the end
	while (this->beingDisposed != true)
	{
		std::function<void()> acceptedTask = nullptr;
		// try to get a task
		{
			std::unique_lock<std::mutex> lock(this->mutex); // aquire lock with lock guard 
			// is master is modifying structure, need to wait until its done
			this->signalMasterDone.wait(lock, [&] { return this->workers_enabled == true; }); // wait until master finishes adding new tasks
			this->workingThreads++;
			lock.unlock(); // let other workes look for tasks

			// actually look for task
			// first try to get a task
			for (taskGroupPtr = this->tasks.begin(); taskGroupPtr != this->tasks.end(); ++taskGroupPtr)
			{
				auto& taskGroup = taskGroupPtr;
				if (taskGroup->mutex.try_lock())
				{
					if (taskGroup->workingThreads < taskGroup->MaxDegreeOfParallelism)
					{
						acceptedTask = taskGroup->Children.front();
						taskGroup->Children.pop();
						taskGroup->workingThreads++;
						taskGroup->mutex.unlock();
						break; // found task stop search
					}
					else
					{
						taskGroup->mutex.unlock();
					}
				}
			}
			// get a task
			// then really get a task
			for (taskGroupPtr = this->tasks.begin(); taskGroupPtr != this->tasks.end(); ++taskGroupPtr)
			{
				auto taskGroup = taskGroupPtr;
				taskGroup->mutex.lock();
				// this time, lock
				if (taskGroup->workingThreads < taskGroup->MaxDegreeOfParallelism)
				{
					acceptedTask = taskGroup->Children.front();
					taskGroup->Children.pop();
					taskGroup->workingThreads++;
					taskGroup->mutex.unlock();
					break; // found task stop search
				}
				else
				{
					taskGroup->mutex.unlock();
				}
			}

			// done
			lock.lock();
			if (acceptedTask == nullptr)
			{
				// no task to do
				this->workers_enabled = false; // disable workers until new task avaiable
			}
			this->workingThreads--;
			lock.unlock();
			// signal potentially waiting master threads to continue if the need to modify structure (add tasks)
			this->signalWorkerDone.notify_all();
		}

		// check if we have task
		if (acceptedTask != nullptr)
		{
			// run the accepted task
			acceptedTask();
			// all done!
			auto& taskGroup = taskGroupPtr;
			bool notify = false;
			taskGroup->mutex.lock();
			taskGroup->workingThreads--;
			if (taskGroup->workingThreads == 0 && taskGroup->Children.empty() == true)
			{
				notify = taskGroup->done = true;
			}
			taskGroup->mutex.unlock();
			if (notify)
			{
				// if someone is waiting on task group, notify it
				taskGroup->signalTaskDone.notify_all();
			}
		}
	}
}

void Carbon::ThreadPool::SubmitForExecution(std::function<void()> task)
{
	std::array<std::function<void()>, 1> data = { task };
	this->SubmitForExecution(data);
}

Carbon::ThreadPool::ThreadPool() : beingDisposed(false), numberOfThreads(std::thread::hardware_concurrency())
{
	InitializeThreads();
}

Carbon::ThreadPool::ThreadPool(int numberOfThreads) : beingDisposed(false), numberOfThreads(numberOfThreads)
{
	InitializeThreads();
}

Carbon::ThreadPool::~ThreadPool()
{

}

bool Carbon::ParallelTaskGroup::IsDone() const
{
	return this->IsDone();
}

void Carbon::ParallelTaskGroup::WaitUntilDone()
{
	std::unique_lock<std::mutex> lock(this->mutex); // aquire lock with lock guard 
	this->signalTaskDone.wait(lock, [&] { return this->done; }); // wait until done
}

void Carbon::ThreadPool::InitializeThreads()
{
	while(this->threads.size() < numberOfThreads)
	{
		std::thread thread([&]() {this->ThreadMethod(); });
		this->threads.push_back(std::move(thread));
	}
}
