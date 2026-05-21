#pragma once

#include <cstdint>
#include <vector>
#include <iostream>

namespace NanoMatch {

enum class Side : uint8_t {
    Buy = 0,
    Sell = 1
};

struct PriceLevel; // Forward declaration

// Packed to avoid padding.
#pragma pack(push, 1)
struct Order {
    uint64_t order_id;
    uint64_t price;
    uint32_t quantity;
    uint64_t timestamp;
    Side side;

    Order* next = nullptr;
    Order* prev = nullptr;
    // No level pointer here - we use the price to find the level safely
};

struct Trade {
    uint64_t buyer_order_id;
    uint64_t seller_order_id;
    uint64_t price;
    uint32_t quantity;
    uint64_t timestamp;
};
#pragma pack(pop)

/**
 * @brief A simple, high-performance memory pool for Order objects.
 * 
 * Pre-allocates a large slab of memory to avoid the overhead of malloc/free (new/delete)
 * during the matching process. This keeps the engine's execution deterministic and 
 * within the sub-microsecond range.
 */
template <typename T, size_t BlockSize = 100000>
class MemoryPool {
public:
    MemoryPool() {
        expand();
    }

    ~MemoryPool() {
        for (auto block : all_blocks) {
            delete[] block;
        }
    }

    // Allocation is O(1) - just popping from a free list.
    T* allocate() {
        if (free_list.empty()) {
            expand();
        }
        T* ptr = free_list.back();
        free_list.pop_back();
        return ptr;
    }

    // Deallocation is O(1) - just pushing back to the free list.
    void deallocate(T* ptr) {
        free_list.push_back(ptr);
    }

private:
    void expand() {
        T* new_block = new T[BlockSize];
        all_blocks.push_back(new_block);
        for (size_t i = 0; i < BlockSize; ++i) {
            free_list.push_back(&new_block[i]);
        }
    }

    std::vector<T*> free_list;
    std::vector<T*> all_blocks;
};

} // namespace NanoMatch
