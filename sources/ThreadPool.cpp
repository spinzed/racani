#include "ThreadPool.h"

#include <iostream>

ThreadPool::ThreadPool(size_t threads) : stop(false) {
    if (threads < 1) {
        threads = std::thread::hardware_concurrency();
    }
    for (size_t i = 0; i < threads; ++i) {
        workers.emplace_back(&ThreadPool::worker, this);
    }
}

// pocisti
ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers) {
        worker.join();
    }
}

void ThreadPool::worker() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(this->queueMutex);
            this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
            if (this->stop && this->tasks.empty())
                return;
            task = std::move(this->tasks.front());
            this->tasks.pop();
        }
        task();
        {
            std::unique_lock<std::mutex> lock(this->expectedJobsMutex);
            if (--remainingExpectedJobs <= 0) {
                workerSignal.release();
            }
        }
    }
}

void ThreadPool::setJobQueue(int jobs) {
    workerSignal.try_acquire();
    remainingExpectedJobs = jobs;
}

void ThreadPool::wait() { workerSignal.acquire(); }
