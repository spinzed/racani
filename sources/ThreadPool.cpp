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

// template <class F, class... Args> void ThreadPool::enqueue(F &&f, Args &&...args) {
//     {
//         std::unique_lock<std::mutex> lock(queue_mutex);
//
//         if (stop)
//             throw std::runtime_error("enqueue on stopped ThreadPool");
//
//         tasks.emplace([f, args...]() { f(args...); });
//     }
//     condition.notify_one();
// }

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
                workerSignal.unlock();
            }
        }
    }
}

void ThreadPool::setJobQueue(int jobs) {
    workerSignal.unlock();
    workerSignal.lock();
    remainingExpectedJobs = jobs;
}

void ThreadPool::wait() { workerSignal.lock(); }
