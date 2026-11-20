# thread-pool-bench

A simple fixed-size thread pool in C++17. It is tested for correctness and
benchmarked by counting primes, compared against a single-threaded version.

## Files

- `include/thread_pool.hpp` - the thread pool
- `src/test_pool.cpp` - correctness tests
- `src/benchmark.cpp` - single-threaded vs thread pool timing

## Build and run

```
make test                      # run the tests
make bench                     # primes up to 2,000,000 with 8 threads
./build/benchmark 5000000 4    # custom range and thread count
```

## How it works

- The constructor starts N worker threads. Each one runs `worker_loop()`.
- `submit()` locks the mutex, pushes the task onto the queue, unlocks, and
  wakes one worker.
- Each worker waits on the condition variable until there is a task or the
  pool is stopping. Then it takes a task, unlocks, and runs the task outside
  the lock.
- `shutdown()` sets `stop_`, wakes every worker, and joins them. Workers
  finish whatever is left in the queue before they exit. It is safe to call
  twice, and the destructor calls it too.
