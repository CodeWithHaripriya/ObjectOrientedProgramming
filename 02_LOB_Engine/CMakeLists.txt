cmake_minimum_required(VERSION 3.16)
project(LOBEngine VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimization flags for HFT ultra-low latency release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -march=native -Wall -Wextra -pthread)
else()
    add_compile_options(-g -Wall -Wextra -pthread)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(LOB_SOURCES
    src/OrderBook.cpp
)

# 1. Main Demo Executable
add_executable(lob_engine_demo
    src/main.cpp
    ${LOB_SOURCES}
)

target_link_libraries(lob_engine_demo PRIVATE pthread)

# 2. Unit Test Executable
add_executable(test_lob
    tests/TestOrderBook.cpp
    ${LOB_SOURCES}
)

target_link_libraries(test_lob PRIVATE pthread)

# 3. Latency Benchmark Executable
add_executable(lob_benchmark
    benchmarks/BenchmarkLOB.cpp
    ${LOB_SOURCES}
)

target_link_libraries(lob_benchmark PRIVATE pthread)

# CTest registration
enable_testing()
add_test(NAME LOBUnitTest COMMAND test_lob)
