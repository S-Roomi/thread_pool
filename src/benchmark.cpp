// Counts the primes from 0 to range_end, first on one thread and then with the thread pool.
// Usage: ./benchmark <range_end> <num_threads>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>

#include "thread_pool.hpp"

bool is_prime(long n) {
    if (n < 2) {
        return false;
    }

    for (long i = 2; i * i <= n; i++) {
        if (n % i == 0) {
            return false;
        }
    }

    return true;
}

long count_primes_range(long start, long end) {
    long count = 0;

    for (long i = start; i < end; i++) {
        if (is_prime(i)) {
            count++;
        }
    }

    return count;
}

double milliseconds_between(std::chrono::steady_clock::time_point start,
                            std::chrono::steady_clock::time_point end) {
    std::chrono::duration<double, std::milli> elapsed = end - start;
    return elapsed.count();
}

int main(int argc, char** argv) {
    long range_end = 2000000;
    if (argc > 1) {
        range_end = std::atol(argv[1]);
    }

    int num_threads = 8;
    if (argc > 2) {
        num_threads = std::atoi(argv[2]);
    }

    // single thread
    std::chrono::steady_clock::time_point serial_start = std::chrono::steady_clock::now();
    long serial_count = count_primes_range(0, range_end);
    std::chrono::steady_clock::time_point serial_end = std::chrono::steady_clock::now();
    double serial_ms = milliseconds_between(serial_start, serial_end);

    // thread pool
    // atomic because many workers add to it at the same time
    std::atomic<long> parallel_count(0);

    ThreadPool pool(num_threads);
    long chunk_size = range_end / num_threads;

    std::chrono::steady_clock::time_point parallel_start = std::chrono::steady_clock::now();

    for (int i = 0; i < num_threads; i++) {
        long start = i * chunk_size;
        long end = start + chunk_size;

        // the last chunk also takes whatever is left over from the division
        if (i == num_threads - 1) {
            end = range_end;
        }

        pool.submit([start, end, &parallel_count]() {
            long count = count_primes_range(start, end);
            parallel_count += count;
        });
    }

    // shutdown waits for all the tasks to finish
    pool.shutdown();

    std::chrono::steady_clock::time_point parallel_end = std::chrono::steady_clock::now();
    double parallel_ms = milliseconds_between(parallel_start, parallel_end);

    std::cout << "=== Thread Pool Benchmark ===" << std::endl;
    std::cout << "Range:            [0, " << range_end << ")" << std::endl;
    std::cout << "Threads:          " << num_threads << std::endl;
    std::cout << "Serial primes:    " << serial_count << " in " << serial_ms << " ms" << std::endl;
    std::cout << "Parallel primes:  " << parallel_count << " in " << parallel_ms << " ms" << std::endl;

    if (serial_count != parallel_count) {
        std::cout << "MISMATCH: the pool dropped or double counted some work" << std::endl;
        return 1;
    }

    double speedup = serial_ms / parallel_ms;
    std::cout << "Speedup:          " << speedup << "x" << std::endl;

    return 0;
}
