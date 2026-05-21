#pragma once

#include <cstddef>
#include <array>
#include <atomic>
#include <new>

namespace NanoMatch {

/**
 * @brief A cache-friendly, fixed-size ring buffer.
 * 
 * Uses alignas(64) to ensure the head and tail pointers are on separate 
 * cache lines (typical x86 cache line size is 64 bytes). This prevents 
 * "False Sharing" where two threads modify different variables that 
 * happen to reside on the same cache line, forcing unnecessary cache 
 * invalidations.
 */
template <typename T, size_t Capacity>
class RingBuffer {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2 for optimization");

public:
    RingBuffer() : head(0), tail(0) {}

    // Push an item onto the buffer. Returns false if full.
    bool push(const T& item) {
        size_t current_tail = tail.load(std::memory_order_relaxed);
        size_t next_tail = (current_tail + 1) & mask;
        
        if (next_tail == head.load(std::memory_order_acquire)) {
            return false; // Buffer full
        }
        
        data[current_tail] = item;
        tail.store(next_tail, std::memory_order_release);
        return true;
    }

    // Pop an item from the buffer. Returns false if empty.
    bool pop(T& item) {
        size_t current_head = head.load(std::memory_order_relaxed);
        
        if (current_head == tail.load(std::memory_order_acquire)) {
            return false; // Buffer empty
        }
        
        item = data[current_head];
        head.store((current_head + 1) & mask, std::memory_order_release);
        return true;
    }

    size_t size() const {
        size_t h = head.load(std::memory_order_relaxed);
        size_t t = tail.load(std::memory_order_relaxed);
        if (t >= h) return t - h;
        return Capacity - (h - t);
    }

private:
    static constexpr size_t mask = Capacity - 1;

    // Cache line padding to avoid false sharing
    alignas(64) std::atomic<size_t> head;
    alignas(64) std::atomic<size_t> tail;
    
    // The data array itself. In a real SPSC queue, we might also 
    // align this to cache lines depending on the use case.
    std::array<T, Capacity> data;
};

} // namespace NanoMatch
