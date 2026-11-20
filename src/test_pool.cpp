#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

#include "thread_pool.hpp"

// submit 1000 tasks and check that every one of them ran
bool test_all_tasks_run() {
    std::atomic<int> counter(0);

    ThreadPool pool(4);
    for (int i = 0; i < 1000; i++) {
        pool.submit([&counter]() {
            counter++;
        });
    }
    pool.shutdown();

    if (counter != 1000) {
        std::cout << "FAIL: expected 1000 tasks to run, got " << counter << std::endl;
        return false;
    }

    std::cout << "PASS: all 1000 tasks ran exactly once" << std::endl;
    return true;
}

// let the destructor shut the pool down while it still has work
// if this test hangs or crashes, the destructor is broken
bool test_destructor_with_pending_work() {
    std::atomic<int> counter(0);

    {
        ThreadPool pool(2);
        for (int i = 0; i < 50; i++) {
            pool.submit([&counter]() {
                counter++;
            });
        }
        // no shutdown() here, the destructor runs at the closing brace
    }

    std::cout << "PASS: destructor completed without hanging (ran " << counter << "/50)" << std::endl;
    return true;
}

// 4 threads all submit tasks to the same pool at the same time
bool test_concurrent_submit() {
    std::atomic<int> counter(0);

    ThreadPool pool(4);

    std::vector<std::thread> submitters;
    for (int t = 0; t < 4; t++) {
        submitters.push_back(std::thread([&pool, &counter]() {
            for (int i = 0; i < 100; i++) {
                pool.submit([&counter]() {
                    counter++;
                });
            }
        }));
    }

    for (size_t i = 0; i < submitters.size(); i++) {
        submitters[i].join();
    }

    pool.shutdown();

    if (counter != 400) {
        std::cout << "FAIL: expected 400 tasks, got " << counter << std::endl;
        return false;
    }

    std::cout << "PASS: concurrent submission from 4 threads, all 400 tasks ran" << std::endl;
    return true;
}

int main() {
    bool all_ok = true;

    if (!test_all_tasks_run()) {
        all_ok = false;
    }

    if (!test_destructor_with_pending_work()) {
        all_ok = false;
    }

    if (!test_concurrent_submit()) {
        all_ok = false;
    }

    if (all_ok) {
        std::cout << std::endl << "ALL TESTS PASSED" << std::endl;
        return 0;
    } else {
        std::cout << std::endl << "SOME TESTS FAILED" << std::endl;
        return 1;
    }
}
