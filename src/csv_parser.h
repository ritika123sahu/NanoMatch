#pragma once

#include <string>
#include <functional>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <charconv>
#include <iostream>
#include <string_view>
#include "order.h"

namespace NanoMatch {

class CSVParser {
public:
    using OrderHandler = std::function<void(uint64_t ts, Side side, uint64_t price, uint32_t qty, char type)>;

    CSVParser(OrderHandler handler) : order_handler_(std::move(handler)) {}

    void parse(const std::string& filename) {
        int fd = open(filename.c_str(), O_RDONLY);
        if (fd == -1) {
            std::cerr << "DEBUG ERROR: Could not open file: " << filename << std::endl;
            return;
        }

        struct stat sb;
        fstat(fd, &sb);
        size_t length = sb.st_size;
        std::cout << "DEBUG: File size is " << length << " bytes." << std::endl;

        void* addr = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);
        if (addr == MAP_FAILED) {
            std::cerr << "DEBUG ERROR: mmap failed" << std::endl;
            close(fd);
            return;
        }

        const char* buffer = static_cast<const char*>(addr);
        const char* end = buffer + length;
        const char* ptr = buffer;

        if (length > 50) {
            std::cout << "DEBUG: First 50 chars: [" << std::string_view(buffer, 50) << "]" << std::endl;
        }

        // Skip header
        std::cout << "DEBUG: Skipping header..." << std::endl;
        while (ptr < end && *ptr != '\n') ptr++;
        if (ptr < end) ptr++;
        std::cout << "DEBUG: Header skipped. Starting loop..." << std::endl;

        uint64_t count = 0;
        while (ptr < end) [[likely]] {
            uint64_t ts = 0, price = 0;
            uint32_t qty = 0;
            Side side;
            char type;

            // 1. Timestamp
            auto res = std::from_chars(ptr, end, ts);
            if (res.ec != std::errc()) {
                if (ptr < end) {
                    std::cerr << "DEBUG ERROR: Failed to parse TS at char '" << *ptr << "' (offset " << (ptr - buffer) << ")" << std::endl;
                }
                break;
            }
            ptr = res.ptr;
            if (*ptr == ',') ptr++; 

            // 2. Side
            if (ptr >= end) break;
            side = (*ptr == 'B') ? Side::Buy : Side::Sell;
            ptr++; // Skip 'B' or 'S'
            if (*ptr == ',') ptr++; 

            // 3. Price
            res = std::from_chars(ptr, end, price);
            if (res.ec != std::errc()) {
                std::cerr << "DEBUG ERROR: Failed to parse Price at offset " << (ptr - buffer) << std::endl;
                break;
            }
            ptr = res.ptr;
            if (*ptr == ',') ptr++;

            // 4. Quantity
            res = std::from_chars(ptr, end, qty);
            if (res.ec != std::errc()) {
                std::cerr << "DEBUG ERROR: Failed to parse Qty at offset " << (ptr - buffer) << std::endl;
                break;
            }
            ptr = res.ptr;
            if (*ptr == ',') ptr++;

            // 5. Type
            if (ptr >= end) break;
            type = *ptr;
            
            order_handler_(ts, side, price, qty, type);
            count++;

            if (count == 1) {
                std::cout << "DEBUG: Successfully parsed first order!" << std::endl;
            }

            // Move to next line
            while (ptr < end && *ptr != '\n') ptr++;
            if (ptr < end) ptr++;
        }

        std::cout << "DEBUG: Loop finished. Total lines parsed: " << count << std::endl;

        munmap(addr, length);
        close(fd);
    }

private:
    OrderHandler order_handler_;
};

} // namespace NanoMatch
