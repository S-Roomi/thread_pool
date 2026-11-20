#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
public:
    ThreadPool(size_t num_threads) {
        stop_ = false;

        for (size_t i = 0; i < num_threads; i++) {
            // each worker thread runs worker_loop() until the pool shuts down
            workers_.push_back(std::thread([this]() {
                worker_loop();
            }));
        }
    }

    // a thread pool should not be copied
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ~ThreadPool() {
        shutdown();
    }

    void submit(std::function<void()> task) {
        queue_mutex_.lock();

        if (stop_) {
            queue_mutex_.unlock();
            return;
        }

        tasks_.push(task);
        queue_mutex_.unlock();

        // wake up one worker to run the task
        cv_.notify_one();
    }

    void shutdown() {
        queue_mutex_.lock();
        stop_ = true;
        queue_mutex_.unlock();

        // wake up every worker so they can see stop_ is true
        cv_.notify_all();

        for (size_t i = 0; i < workers_.size(); i++) {
            // joinable() is false if we already joined, so calling shutdown twice is ok
            if (workers_[i].joinable()) {
                workers_[i].join();
            }
        }
    }

    size_t pending() {
        queue_mutex_.lock();
        size_t count = tasks_.size();
        queue_mutex_.unlock();
        return count;
    }

private:
    void worker_loop() {
        while (true) {
            std::unique_lock<std::mutex> lock(queue_mutex_);

            // wait() unlocks the mutex while sleeping and locks it again when it wakes up
            while (tasks_.empty() && !stop_) {
                cv_.wait(lock);
            }

            if (tasks_.empty() && stop_) {
                return;
            }

            std::function<void()> task = tasks_.front();
            tasks_.pop();
            lock.unlock();

            // run the task without holding the lock so other workers can grab tasks
            task();
        }
    }

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;
    bool stop_;
};
