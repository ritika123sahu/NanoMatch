#include "order_book.h"
#include "stl_order_book.h"
#include "performance_timer.h"
#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>

using namespace NanoMatch;

void run_bench() {
    const int N = 1000000;
    PerformanceTimer timer;
    
    // 1. Optimized Version
    OrderBook opt_book;
    std::vector<uint64_t> opt_latencies;
    opt_latencies.reserve(N);

    // Aggressive Warmup
    for(int i=0; i<100000; ++i) {
        opt_book.limit_order(i, Side::Buy, 1000 + (i % 500), 10);
        opt_book.limit_order(i + 100000, Side::Sell, 1000 + (i % 500), 10);
    }

    for(int i=0; i<N; ++i) {
        uint64_t s = timer.now();
        if (i % 2 == 0) {
            opt_book.limit_order(200000+i, Side::Sell, 1000 + (i % 100), 1);
        } else {
            opt_book.limit_order(200000+i, Side::Buy, 900 + (i % 100), 1);
        }
        uint64_t e = timer.now();
        opt_latencies.push_back(timer.to_nanoseconds(e - s));
    }

    // 2. STL Baseline
    STLOrderBook stl_book;
    std::vector<uint64_t> stl_latencies;
    stl_latencies.reserve(N);

    for(int i=0; i<100000; ++i) {
        stl_book.limit_order(i, Side::Buy, 1000 + (i % 500), 10);
        stl_book.limit_order(i + 100000, Side::Sell, 1000 + (i % 500), 10);
    }

    for(int i=0; i<N; ++i) {
        uint64_t s = timer.now();
        if (i % 2 == 0) {
            stl_book.limit_order(200000+i, Side::Sell, 1000 + (i % 100), 1);
        } else {
            stl_book.limit_order(200000+i, Side::Buy, 900 + (i % 100), 1);
        }
        uint64_t e = timer.now();
        stl_latencies.push_back(timer.to_nanoseconds(e - s));
    }

    auto get_stats = [](std::vector<uint64_t>& v) {
        std::sort(v.begin(), v.end());
        return std::make_pair(v[v.size()/2], v[v.size()*0.99]);
    };

    auto [opt_p50, opt_p99] = get_stats(opt_latencies);
    auto [stl_p50, stl_p99] = get_stats(stl_latencies);

    std::cout << "\n==========================================" << std::endl;
    std::cout << " PERFORMANCE COMPARISON (Baseline vs Optimized)" << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << " Metric    | STL Baseline | NanoMatch (Opt) | Improvement" << std::endl;
    std::cout << "-----------|--------------|-----------------|------------" << std::endl;
    std::cout << " P50 Latency | " << std::setw(10) << stl_p50 << "ns | " << std::setw(13) << opt_p50 << "ns | " 
              << std::fixed << std::setprecision(1) << (double)stl_p50/opt_p50 << "x fast" << std::endl;
    std::cout << " P99 Latency | " << std::setw(10) << stl_p99 << "ns | " << std::setw(13) << opt_p99 << "ns | " 
              << (double)stl_p99/opt_p99 << "x fast" << std::endl;
    std::cout << "==========================================\n" << std::endl;
}

int main() {
    run_bench();
    return 0;
}
