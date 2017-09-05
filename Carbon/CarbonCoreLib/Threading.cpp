#include "Threading.h"
#include <array>
#include <iostream>
#include <string>

void Carbon::ParallelTaskGroup::AddTask(std::function<void()> task)
{
	this->mutex.lock();
	this->Children.push(task);
	this->mutex.unlock();
}

void Carbon::ThreadPool::ThreadMethod(int threadId)
{
	ParallelTaskGroup* taskGroupPtr = nullptr;
	// check for the end
	while (this->beingDisposed != true)
	{
		std::function<void()> acceptedTask = nullptr;
		// try to get a task
		{
			//std::cerr << std::string("worker")+std::to_string(threadId)+" lock pool\n";
			std::unique_lock<std::mutex> lock(this->mutex); // aquire lock with lock guard 
			// is master is modifying structure, need to wait until its done
			//std::cerr << std::string("worker")+std::to_string(threadId)+"signalMasterDone.wait\n";
			this->signalMasterDone.wait(lock, [&] { return this->workers_enabled == true; }); // wait until master finishes adding new tasks
			//std::cerr << std::string("worker")+std::to_string(threadId)+"pool workingThreads++\n";
			this->workingThreads++;
			//std::cerr << std::string("worker")+std::to_string(threadId)+" unlock pool\n";
			lock.unlock(); // let other workes look for tasks

			bool anyTasksLeft = false;

			// actually look for task
			// first try to get a task
			/*for (auto taskGroupIterator = this->tasks.begin(); taskGroupIterator != this->tasks.end(); ++taskGroupIterator)
			{
				auto& taskGroup = taskGroupIterator;
				if (taskGroup->mutex.try_lock())
				{
					std::cerr << std::string("worker")+std::to_string(threadId)+" locked group\n";
					anyTasksLeft |= !taskGroup->Children.empty();
					if (taskGroup->workingThreads < taskGroup->MaxDegreeOfParallelism)
					{
						if (!taskGroup->Children.empty())
						{
							acceptedTask = taskGroup->Children.front();
							taskGroupPtr = &*taskGroup;
							taskGroup->Children.pop();
							taskGroup->workingThreads++;
						}
						std::cerr << std::string("worker")+std::to_string(threadId)+" unlock group\n";
						taskGroup->mutex.unlock();
						break; // found task stop search
					}
					else
					{
						std::cerr << std::string("worker")+std::to_string(threadId)+" unlock group\n";
						taskGroup->mutex.unlock();
					}
				}
			}*/
			// get a task
			// then really get a task
			for (auto taskGroupIterator = this->tasks.begin(); taskGroupIterator != this->tasks.end(); ++taskGroupIterator)
			{
				auto taskGroup = taskGroupIterator;
				//std::cerr << std::string("worker")+std::to_string(threadId)+"lock group (" + std::to_string(taskGroup->Children.size()) +  ") tasks \n";
				taskGroup->mutex.lock();
				// this time, lock
				anyTasksLeft |= !taskGroup->Children.empty();
				if (taskGroup->workingThreads < taskGroup->MaxDegreeOfParallelism)
				{
					if (!taskGroup->Children.empty())
					{
						acceptedTask = taskGroup->Children.front();
						taskGroupPtr = &*taskGroup;
						taskGroup->Children.pop();
						//std::cerr << std::string("worker") + std::to_string(threadId) + " AQUIRED task (" + std::to_string(taskGroup->Children.size()) +") left\n";
						//std::cerr << std::string("worker")+std::to_string(threadId)+"group workingThreads++\n";
						taskGroup->workingThreads++;
					}
					//std::cerr << std::string("worker")+std::to_string(threadId)+" unlock group\n";
					taskGroup->mutex.unlock();
					break; // found task stop search
				}
				else
				{
					//std::cerr << std::string("worker")+std::to_string(threadId)+" unlock group\n";
					taskGroup->mutex.unlock();
				}
			}

			// done
			//std::cerr << std::string("worker") + std::to_string(threadId) + " lock pool\n";
			lock.lock();
			if (anyTasksLeft == false && acceptedTask == nullptr)
			{
				// no task to do
				this->workers_enabled = false; // disable workers until new task avaiable
				//std::cerr << std::string("worker")+std::to_string(threadId)+" found no task, disabling workers\n";
			}
			//std::cerr << std::string("worker")+std::to_string(threadId)+"pool workingThreads--\n";
			this->workingThreads--;
			//std::cerr << std::string("worker")+std::to_string(threadId)+" unlock pool\n";
			lock.unlock();
			// signal potentially waiting master threads to continue if the need to modify structure (add tasks)
			//std::cerr << std::string("worker")+std::to_string(threadId)+"notify all signalWorkerDone\n";
			this->signalWorkerDone.notify_all();
		}

		// check if we have task
		if (acceptedTask != nullptr)
		{
			// run the accepted task
			//std::cerr << std::string("worker")+std::to_string(threadId)+" EXECUTE TASK\n";
			acceptedTask();
			// all done!
			auto& taskGroup = taskGroupPtr;
			bool notify = false;
			//std::cerr << std::string("worker")+std::to_string(threadId)+" lock group\n";
			taskGroup->mutex.lock();
			//std::cerr << std::string("worker")+std::to_string(threadId)+"group workingThreads--\n";
			taskGroup->workingThreads--;
			if (taskGroup->workingThreads == 0 && taskGroup->Children.empty() == true)
			{
				notify = taskGroup->done = true;
			}
			//std::cerr << std::string("worker")+std::to_string(threadId)+" unlock group\n";
			taskGroup->mutex.unlock();
			if (notify)
			{
				// if someone is waiting on task group, notify it
				//std::cerr << std::string("worker")+std::to_string(threadId)+" notify group signalTaskDone\n";
				taskGroup->signalTaskDone.notify_all();
			}
		}
	}
}

Carbon::ParallelTaskGroup* Carbon::ThreadPool::SubmitForExecution(std::function<void()> task)
{
	std::array<std::function<void()>, 1> data = { task };
	return this->SubmitForExecution(data);
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
	return this->done;
}

void Carbon::ParallelTaskGroup::WaitUntilDone()
{
	//std::cerr << "master lock group\n";
	std::unique_lock<std::mutex> lock(this->mutex); // aquire lock with lock guard 
	this->signalTaskDone.wait(lock, [&] { return this->done; }); // wait until done
	//std::cerr << "master unlock group\n";
}

void Carbon::ThreadPool::InitializeThreads()
{
	while(this->threads.size() < numberOfThreads)
	{
		int id = this->threads.size();
		std::thread thread([this, id]() {this->ThreadMethod(id); });
		this->threads.push_back(std::move(thread));
	}
}
