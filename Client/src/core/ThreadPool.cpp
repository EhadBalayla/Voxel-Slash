#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t count) {
    for (size_t i = 0; i < count; i++) {
		threads.emplace_back(std::thread(&ThreadPool::ThreadLoop, this));
	}
}
ThreadPool::ThreadPool() : ThreadPool(3) {}
void ThreadPool::Stop() {
    ThreadsRunning = false;

	cv.notify_all();

	for (size_t i = 0; i < threads.size(); i++) {
		threads[i].join();
	}
}

void ThreadPool::QueueJob(job j) {
	{
		std::lock_guard<std::mutex> lock(jobs_mutex);
		jobs_queue.push(j);
	}
	cv.notify_one();
}

void ThreadPool::ThreadLoop() {
    while(ThreadsRunning) {
        job j;
        {
            std::unique_lock<std::mutex> lock(jobs_mutex);
            cv.wait(lock, [this]{ return !jobs_queue.empty() || !ThreadsRunning; });

            if(!ThreadsRunning) break;

            j = jobs_queue.front();
            jobs_queue.pop();
        }

        j.func(j.data1, j.data2);
    }
}