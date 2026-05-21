# NanoMatch: Ultra-Low Latency Order Matching Engine

NanoMatch is a high-performance Limit Order Book (LOB) built in C++20, designed for sub-microsecond execution. It prioritizes "Hardware Sympathy"—aligning software architecture with CPU cache hierarchies and memory management behaviors to achieve deterministic, ultra-low latency.

## Performance Snapshot (macOS M1/M2/M3)
- **P50 Latency:** ~125 ns
- **P99 Latency:** ~166 ns
- **Throughput:** ~7M+ orders/sec (depending on matching density)

## Architectural Core

### 1. Zero-Allocation Hot Path
Dynamic memory allocation (`malloc`/`new`) is a latency killer due to non-deterministic syscalls and heap fragmentation. NanoMatch uses a **pre-allocated Memory Pool** for all `Order` objects. 
- **Complexity:** $O(1)$ allocation/deallocation via a lock-free free list.
- **Impact:** Zero heap churn during active matching.

### 2. Cache-Optimized Data Structures
- **Intrusive Doubly-Linked Lists:** Orders at each price level are stored in an intrusive list. This allows for $O(1)$ cancellation and fill-removal without extra pointer chasing or container overhead.
- **Struct Packing:** The `Order` struct is packed using `#pragma pack(1)` to fit as much data as possible into a single 64-byte L1 cache line.
- **Flat Price Levels:** Bid and Ask levels are stored in sorted contiguous vectors, making "Best Bid/Offer" (BBO) lookups extremely cache-friendly.

### 3. Lock-Free Trade Logging
To prevent I/O blocking the matching engine, trades are offloaded via a **Single-Producer Single-Consumer (SPSC) Ring Buffer**.
- **Hardware Sympathy:** Uses `alignas(64)` to prevent **False Sharing** by ensuring the head and tail pointers reside on separate cache lines.
- **Memory Ordering:** Uses `std::memory_order_acquire/release` semantics for efficient thread synchronization without mutexes.

### 4. Zero-Copy Ingestion
- **ITCH 5.0 Parser:** Uses `mmap` to map binary market data files directly into the process address space. Data is parsed "in-place" without copying into intermediate buffers.
- **Branch Prediction:** Critical paths use `[[likely]]` and `[[unlikely]]` hints to guide the CPU's branch predictor, minimizing pipeline stalls.

## Build and Run

### Prerequisites
- CMake 3.15+
- Clang/LLVM (C++20 support)

### Compilation
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
```

### Benchmarking
To run the latency suite:
```bash
./bench_latencies
```

## Visual Profiling (macOS)
On macOS, use **Instruments** for performance analysis:
1. Open **Instruments.app**.
2. Select the **Time Profiler** template.
3. Target the `bench_latencies` executable.
4. Record.
5. In the bottom-right detail view, select **Call Tree** -> **Show Flame Graph**.
*You will observe that nearly 90% of the time is spent in the matching loop, with almost no time spent in kernel syscalls or memory management.*
