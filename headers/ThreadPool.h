#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

// ThreadPool class to manage worker threads
class ThreadPool {
  public:
    ThreadPool(size_t threads);
    ~ThreadPool();

    // Add new work item to the pool
    // void enqueue(std::function<void()> task);
    // template <class F, class... Args> void enqueue(F &&f, Args &&...args);
    template <class F, typename T, class... Args> void enqueue(F &&f, T *t, Args &&...args) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace([f, t, args...]() { (t->*f)(args...); });
        }
        condition.notify_one();
    }

    void setJobQueue(int jobs);
    void wait();

  private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;

    std::mutex expectedJobsMutex;
    std::mutex workerSignal;
    int remainingExpectedJobs;

    bool stop;

    // Worker function
    void worker();
};
