#include "order_book.h"
#include "performance_timer.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

using namespace NanoMatch;

void run_latency_test(int num_orders) {
    OrderBook book;
    PerformanceTimer timer;
    std::vector<uint64_t> latencies;
    latencies.reserve(num_orders);

    // Warm up
    for (int i = 0; i < 1000; ++i) {
        book.limit_order(i, Side::Buy, 100, 1);
    }

    std::cout << "Starting latency test with " << num_orders << " orders..." << std::endl;

    for (int i = 0; i < num_orders; ++i) {
        uint64_t start = timer.now();
        book.limit_order(10000 + i, Side::Sell, 100, 1); // Matches immediately
        uint64_t end = timer.now();
        latencies.push_back(timer.to_nanoseconds(end - start));
    }

    std::sort(latencies.begin(), latencies.end());

    double avg = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
    uint64_t p50 = latencies[latencies.size() / 2];
    uint64_t p90 = latencies[latencies.size() * 0.9];
    uint64_t p99 = latencies[latencies.size() * 0.99];
    uint64_t p999 = latencies[latencies.size() * 0.999];

    std::cout << "\n--- LATENCY RESULTS (ns) ---" << std::endl;
    std::cout << "Average: " << avg << std::endl;
    std::cout << "P50:     " << p50 << std::endl;
    std::cout << "P90:     " << p90 << std::endl;
    std::cout << "P99:     " << p99 << std::endl;
    std::cout << "P99.9:   " << p999 << std::endl;
    std::cout << "----------------------------\n" << std::endl;
}

int main() {
    run_latency_test(100000);
    return 0;
}
