# 📄 CV / Resume Bullet Points for Project 1: AsyncNet-Server

Copy and paste these bullet points directly into your Resume / CV under **Projects** or **Technical Experience**:

---

### Option A (Action + Impact Focused - Recommended):
- **AsyncNet-Server (High-Performance C++ Network Engine)** | *Boost.Asio, C++17/20, Modern CMake, Multithreading*
  - Engineered an asynchronous multi-threaded TCP socket server in C++ using **Boost.Asio**, achieving **86,000+ req/sec** throughput with sub-300 µs average round-trip latency across 25 concurrent client connections.
  - Implemented a custom 8-byte **Type-Length-Value (TLV) binary protocol** with network byte order serialization and framing validation to protect against partial reads and buffer overflows.
  - Designed thread-safe session handlers using **`boost::asio::strand`** and **`std::enable_shared_from_this`**, ensuring race-condition-free event dispatching and safe object lifetime management under concurrent disconnections.
  - Developed an integrated multi-threaded load generator and CTest unit testing suite, measuring p50, p95, and p99 latency distributions.

---

### Option B (Concise / Standard 3-Bullet Format):
- Built a multi-threaded asynchronous TCP network server in C++17/20 using **Boost.Asio** capable of handling 86,000+ requests/sec with average latencies under 265 µs.
- Architected a custom TLV binary messaging protocol with big-endian network byte ordering and dynamic packet buffer pooling.
- Managed concurrency safety and object lifetime across worker threads using `boost::asio::strand` and RAII shared pointer semantics.
