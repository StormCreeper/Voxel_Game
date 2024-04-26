#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

/**
 * Code from https://stackoverflow.com/questions/15752659/thread-pooling-in-c11
 */

#include <mutex>
#include <thread>
#include <condition_variable>
#include <vector>
#include <queue>
#include <functional>

class ThreadPool {
   public:
    void Start();
    void QueueJob(const std::function<void()>& job);
    void Stop();
    bool busy();

   private:
    void ThreadLoop();

    bool should_terminate = false;            // Tells threads to stop looking for jobs
    std::mutex queue_mutex;                   // Prevents data races to the job queue
    std::condition_variable mutex_condition;  // Allows threads to wait on new jobs or termination
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> jobs;
};

#endif  // THREAD_POOL_HPP