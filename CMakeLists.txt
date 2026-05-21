cmake_minimum_required(VERSION 3.15)
project(NanoMatch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Performance flags
if(MSVC)
    add_compile_options(/W4 /O2 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3 -march=native -fomit-frame-pointer)
endif()

# Directories
include_directories(src)

# Google Benchmark (Optional but recommended for Phase 5)
# In a real scenario, you'd find_package or FetchContent
# For now, we'll focus on the core logic.

# Main Engine Executable
add_executable(nanomatch src/matching_engine.cpp src/order_book.cpp src/itch_parser.cpp)

# Benchmarks
add_executable(bench_latencies bench/bench_latencies.cpp src/order_book.cpp src/itch_parser.cpp)
add_executable(bench_comparison bench/bench_comparison.cpp src/order_book.cpp src/itch_parser.cpp)
