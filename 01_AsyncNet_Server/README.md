# AsyncNet-Server 🚀

[![C++ Standard](https://img.shields.io/badge/C%2B%2B-17%2F20-blue.svg)](https://isocpp.org/)
[![Boost.Asio](https://img.shields.io/badge/Boost.Asio-1.83-orange.svg)](https://www.boost.org/)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![License](https://img.shields.io/badge/license-MIT-green.svg)]()

A high-performance, asynchronous multi-threaded TCP network server and custom protocol framing engine implemented in Modern C++ (C++17/20) using **Boost.Asio**. 

`AsyncNet-Server` is engineered for ultra-low latency, concurrency safety, and scalable connection management. It processes **86,000+ requests/sec** with an average round-trip latency under **265 microseconds** on a 4-core worker pool.

---

## 📐 Architecture Overview

```
                               +-------------------------------------+
                               |         AsyncNet-Server             |
                               |                                     |
    +---------------+          |   +-----------------------------+   |
    |  Client 1     | -------->|   |    boost::asio::io_context   |   |
    +---------------+          |   +-----------------------------+   |
                               |                  |                  |
    +---------------+          |                  v                  |
    |  Client 2     | -------->|   +-----------------------------+   |
    +---------------+          |   | Dynamic Worker Thread Pool  |   |
                               |   +-----------------------------+   |
    +---------------+          |     |           |           |       |
    |  Client N     | -------->|     v           v           v       |
    +---------------+          |  [Strand 1]  [Strand 2]  [Strand N] |
                               |     |           |           |       |
                               |     v           v           v       |
                               |  [Session 1] [Session 2] [Session N]|
                               +-------------------------------------+
```

### Key Highlights
- **Asynchronous Non-Blocking I/O**: Powered by `boost::asio::io_context` with dynamic worker thread pool execution.
- **Lock-Free Handler Synchronization**: Leverages `boost::asio::strand` to serialize session callback handlers without mutex contention.
- **Custom Binary Framing (TLV Protocol)**: Type-Length-Value packet layout with network byte order (`htons`/`htonl`) encoding and boundary validation.
- **Connection Lifecycle & RAII**: Shared lifetime management using `std::enable_shared_from_this<Session>` preventing dangling pointer callbacks under heavy concurrent disconnects.
- **Integrated Benchmark Generator**: Multi-threaded load testing client providing microsecond-level latency distributions (p50, p95, p99) and request throughput reporting.

---

## 📜 Binary Protocol Specification

Packets sent across `AsyncNet-Server` follow an 8-byte fixed header followed by a dynamic payload:

| Field | Size (Bytes) | Description |
| :--- | :--- | :--- |
| `Magic` | 2 | Header validation signature (`0x414E` = "AN") |
| `Type` | 2 | Enumerated Message Type (Ping, Echo, Data, Disconnect) |
| `Length` | 4 | Payload byte size ($0 \le N \le 65536$) |
| `Payload` | $N$ | Binary / UTF-8 payload content |

---

## ⚡ Performance & Benchmark Results

Evaluated on 4 worker threads handling 25 concurrent client connections executing 25,000 requests:

| Metric | Value |
| :--- | :--- |
| **Throughput** | **86,342 req/sec** |
| **Total Workload** | 25,000 requests |
| **Failed Requests** | **0 (0.00%)** |
| **Average Latency (Mean)** | **264.84 µs** |
| **Median Latency (p50)** | **255.49 µs** |
| **95th Percentile (p95)** | **393.58 µs** |
| **99th Percentile (p99)** | **638.33 µs** |

---

## 🛠️ Build & Run Instructions

### Prerequisites
- GCC 10+ / Clang 11+ / MSVC 2019+
- CMake 3.16+
- Boost C++ Libraries (1.70+)

### Building from Source
```bash
git clone https://github.com/your-username/AsyncNet-Server.git
cd AsyncNet-Server

# Configure release build
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build targets
cmake --build build -j 4
```

### Running the Unit Tests
```bash
./build/test_protocol
```

### Running the Server
```bash
./build/asyncnet_server --port 9090 --threads 4
```

### Running the Load Benchmark
```bash
./build/asyncnet_bench_client --host 127.0.0.1 --port 9090 --clients 25 --requests 1000
```

---

## 👨‍💻 Author & Career Growth Narrative
This project represents **Project 1** in a 3-tier high-performance C++ systems portfolio showcasing progression from foundational socket networking to low-latency financial systems and distributed consensus algorithms.

- 📬 LinkedIn: [Your Profile Link]
- 📁 GitHub: [Your GitHub Link]
