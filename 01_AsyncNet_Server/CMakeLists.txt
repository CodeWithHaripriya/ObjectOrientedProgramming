cmake_minimum_required(VERSION 3.16)
project(AsyncNetServer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimization flags for Release build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -Wall -Wextra -pthread)
else()
    add_compile_options(-g -Wall -Wextra -pthread)
endif()

# Find Boost headers
find_package(Boost 1.70 REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
)

# Common library source files
set(SERVER_SOURCES
    src/Protocol.cpp
    src/SessionManager.cpp
    src/Session.cpp
    src/Server.cpp
)

# 1. Main Server Executable
add_executable(asyncnet_server
    src/main.cpp
    ${SERVER_SOURCES}
)

target_link_libraries(asyncnet_server
    PRIVATE
    ${Boost_LIBRARIES}
    pthread
)

# 2. Benchmark Client Executable
add_executable(asyncnet_bench_client
    client/BenchmarkClient.cpp
    src/Protocol.cpp
)

target_link_libraries(asyncnet_bench_client
    PRIVATE
    ${Boost_LIBRARIES}
    pthread
)

# 3. Protocol Unit Tests Executable
add_executable(test_protocol
    tests/TestProtocol.cpp
    src/Protocol.cpp
)

target_link_libraries(test_protocol
    PRIVATE
    ${Boost_LIBRARIES}
    pthread
)

# Register CTest
enable_testing()
add_test(NAME ProtocolUnitTest COMMAND test_protocol)
