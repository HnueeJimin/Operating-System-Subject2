#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <cstdint>
#include "queue.h"

using namespace std;

int main() {
    const int num_threads = 32;
    const int64_t total_keys = 10000000;
    const int64_t keys_per_thread = total_keys / num_threads;

    // Initialize queue
    Queue* q = init();

    // Start timer
    auto start = chrono::steady_clock::now();

    // Launch enqueue threads
    vector<thread> threads;
    threads.reserve(num_threads);
    for (int i = 0; i < num_threads; ++i) {
        int64_t begin = i * keys_per_thread;
        int64_t end = (i == num_threads - 1) ? total_keys : (i + 1) * keys_per_thread;
        threads.emplace_back([q, begin, end]() {
            for (int64_t key = begin; key < end; ++key) {
                Item item = { (Key)key, nullptr, 0 };
                enqueue(q, item);
            }
            });
    }

    // Join threads
    for (auto& t : threads) {
        t.join();
    }

    // Stop timer
    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    // Output total elapsed time in milliseconds
    cout << duration << " ms" << endl;

    // Cleanup
    release(q);
    return 0;
}
