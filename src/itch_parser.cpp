#include "itch_parser.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <iostream>

namespace NanoMatch {

void ITCHParser::parse(const std::string& filename) {
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
        throw std::runtime_error("Could not open ITCH file");
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        close(fd);
        throw std::runtime_error("Could not stat ITCH file");
    }

    size_t length = sb.st_size;
    // Map the file into memory. Zero-copy: we read directly from the page cache.
    void* addr = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
        close(fd);
        throw std::runtime_error("mmap failed");
    }

    // Advise the kernel that we will read this sequentially to trigger read-ahead.
    #ifdef __linux__
        madvise(addr, length, MADV_SEQUENTIAL | MADV_WILLNEED);
    #else
        madvise(addr, length, MADV_SEQUENTIAL);
    #endif

    const uint8_t* buffer = static_cast<const uint8_t*>(addr);
    size_t offset = 0;

    while (offset < length) [[likely]] {
        // ITCH 5.0 messages are prefixed with a 2-byte length (Big Endian)
        uint16_t msg_len = (buffer[offset] << 8) | buffer[offset + 1];
        offset += 2;

        char msg_type = static_cast<char>(buffer[offset]);
        
        // Pass the raw pointer to the handler. Zero allocations.
        handler_(msg_type, &buffer[offset + 1]);

        offset += msg_len;
    }

    munmap(addr, length);
    close(fd);
}

} // namespace NanoMatch
