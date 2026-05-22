#pragma once

#include <cstdint>
#include <vector>
#include <iostream>

namespace NanoMatch {

enum class Side : uint8_t {
    Buy = 0,
    Sell = 1
};

// Packed and aligned to 32 bytes (half a cache line) to ensure two orders fit in one 64-byte L1 cache line.
struct alignas(32) Order {
    uint64_t order_id;
    uint64_t price;
    uint32_t quantity;
    Side side;
    uint8_t padding[3]; // Explicit padding for alignment
    uint64_t timestamp;

    Order* next;
    Order* prev;
};

struct alignas(64) Trade {
    uint64_t buyer_order_id;
    uint64_t seller_order_id;
    uint64_t price;
    uint32_t quantity;
    uint8_t padding[4];
    uint64_t timestamp;
};

/**
 * @brief Ultra-fast Memory Pool using a simple stack-based free list.
 */
template <typename T, size_t BlockSize = 100000>
class MemoryPool {
public:
    MemoryPool() {
        expand();
    }

    ~MemoryPool() {
        for (auto block : all_blocks) {
            operator delete[](block, std::align_val_t{alignof(T)});
        }
    }

    // Hot path: O(1) pointer swap
    inline T* allocate() {
        if (__builtin_expect(free_ptr == nullptr, 0)) {
            expand();
        }
        T* res = free_ptr;
        free_ptr = *((T**)free_ptr);
        return res;
    }

    // Hot path: O(1) pointer swap
    inline void deallocate(T* ptr) {
        *((T**)ptr) = free_ptr;
        free_ptr = ptr;
    }

private:
    void expand() {
        // Use aligned_alloc for cache-aligned blocks
        void* block = operator new[](sizeof(T) * BlockSize, std::align_val_t{alignof(T)});
        T* new_block = static_cast<T*>(block);
        all_blocks.push_back(new_block);
        
        for (size_t i = 0; i < BlockSize - 1; ++i) {
            *((T**)&new_block[i]) = &new_block[i + 1];
        }
        *((T**)&new_block[BlockSize - 1]) = free_ptr;
        free_ptr = new_block;
    }

    T* free_ptr = nullptr;
    std::vector<T*> all_blocks;
};

} // namespace NanoMatch
