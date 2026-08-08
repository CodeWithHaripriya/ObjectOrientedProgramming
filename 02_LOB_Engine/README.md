# LOB-Engine ⚡

[![C++ Standard](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![Performance](https://img.shields.io/badge/Latency-70ns_p50-brightgreen.svg)]()
[![Throughput](https://img.shields.io/badge/Throughput-7.0M_ops%2Fsec-orange.svg)]()
[![License](https://img.shields.io/badge/license-MIT-green.svg)]()

An ultra-low latency, zero-heap-allocation Limit Order Book (LOB) and Financial Matching Engine implemented in **Modern C++20**.

`LOB-Engine` is engineered for High-Frequency Trading (HFT) environments, processing over **7.0 Million order operations per second** with a median (p50) execution latency of **70 nanoseconds**.

---

## 📐 Architecture & Optimization Highlights

```
                   +-----------------------------------------------+
                   |              LOB-Engine (C++20)               |
                   |                                               |
  Incoming Orders  |   +---------------------------------------+   |
==================>|   | Lock-Free SPSC Ring Buffer Queue      |   |
                   |   | (alignas false-sharing padding)       |   |
                   |   +---------------------------------------+   |
                   |                       |                       |
                   |                       v                       |
                   |   +---------------------------------------+   |
                   |   | Price-Time Priority Matching Core     |   |
                   |   |                                       |   |
                   |   |   Bids (Desc)       Asks (Asc)        |   |
                   |   |  +------------+   +------------+      |   |
                   |   |  | Price 100  |   | Price 101  |      |   |
                   |   |  | [O1]->[O2] |   | [O4]->[O5] |      |   |
                   |   |  +------------+   +------------+      |   |
                   |   +---------------------------------------+   |
                   |                       |                       |
                   |                       v                       |
                   |   +---------------------------------------+   |
                   |   | Fixed-Size Custom Memory Pool         |   |
                   |   | (Zero dynamic heap allocations)       |   |
                   |   +---------------------------------------+   |
                   +-----------------------------------------------+
```

### Low-Latency Systems Design:
1. **Zero Dynamic Allocation in Hot Path**: Uses a pre-allocated `ObjectPool<T>` memory manager for `Order` and `PriceLevel` nodes, eliminating `malloc`/`free` system call latency and OS heap lock contention.
2. **$O(1)$ Price-Time Priority & Instant Cancellation**: Intrusive doubly-linked lists enable $O(1)$ order placement, $O(1)$ top-of-book matching, and $O(1)$ order cancellation by pointer unlinking without tree traversal.
3. **Lock-Free SPSC Command Queue**: Single-Producer Single-Consumer atomic ring buffer padded with `alignas(64)` (`hardware_destructive_interference_size`) to prevent false-sharing cache line invalidation between producer and consumer CPU cores.
4. **Modern C++20 Features**:
   - `std::concepts` for compile-time parameter constraint enforcement (`PriceConcept`, `QuantityConcept`).
   - `std::span` for zero-copy trade execution batching.
   - `[[unlikely]]` branch prediction annotations to optimize CPU pipeline instruction caching.

---

## ⚡ Performance & Benchmark Results

Evaluated across **500,000 order operations** (Add Limit Orders, Aggressive Market Orders, and $O(1)$ Cancellations):

| Metric | Value |
| :--- | :--- |
| **Throughput** | **7,044,381 ops/sec** |
| **Mean Latency** | **108.12 ns (0.11 µs)** |
| **50th Percentile (p50)** | **70.00 ns (0.07 µs)** |
| **90th Percentile (p90)** | **181.00 ns (0.18 µs)** |
| **99th Percentile (p99)** | **441.00 ns (0.44 µs)** |
| **99.9th Percentile (p99.9)** | **1.09 µs** |

---

## 🛠️ Build & Run Instructions

### Prerequisites
- GCC 10+ / Clang 11+ / MSVC 2019+ supporting **C++20**
- CMake 3.16+

### Building from Source
```bash
git clone https://github.com/CodeWithHaripriya/ObjectOrientedProgramming.git
cd ObjectOrientedProgramming/02_LOB_Engine

# Configure release build with C++20 optimizations
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build all targets
cmake --build build -j 4
```

### Running the Interactive Demo
```bash
./build/lob_engine_demo
```

### Running the Unit Test Suite
```bash
./build/test_lob
```

### Running the Nanosecond Latency Benchmark
```bash
./build/lob_benchmark
```

---

## 👨‍💻 Author & Career Growth Narrative
This project represents high-performance C++ systems portfolio showcasing progression from foundational socket networking ([AsyncNet-Server](../01_AsyncNet_Server)) to low-latency financial systems ([LOB-Engine](./)) and distributed consensus engines ([DistriKV-Raft](../03_DistriKV_Raft)).

- 📬 LinkedIn: linkedin.com/in/haripriya-m-a-4b9a151b2
- 📁 GitHub: https://github.com/CodeWithHaripriya/ObjectOrientedProgramming
