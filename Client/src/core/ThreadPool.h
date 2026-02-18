#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

struct job {
    void(*func)(void* p1, void* p2);
    void* data1;
    void* data2;
};

class ThreadPool {
public:
    ThreadPool(size_t count);
    ThreadPool();
    void Stop(); //does the same thing the destructor does

    void QueueJob(job j);
private:
    void ThreadLoop();

    bool ThreadsRunning = true;
    std::vector<std::thread> threads;
    std::mutex jobs_mutex;
    std::queue<job> jobs_queue;
    std::condition_variable cv;
};