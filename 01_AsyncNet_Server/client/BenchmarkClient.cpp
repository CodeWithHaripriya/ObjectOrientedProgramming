#include "Protocol.hpp"
#include "Common.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <numeric>
#include <iomanip>

using boost::asio::ip::tcp;

struct BenchmarkResult {
    uint64_t totalRequests{0};
    uint64_t totalBytesTransferred{0};
    double totalDurationSec{0.0};
    double avgLatencyUs{0.0};
    double p99LatencyUs{0.0};
    uint64_t errors{0};
};

void runClientWorker(
    const std::string& host,
    uint16_t port,
    int requestsPerClient,
    std::atomic<uint64_t>& globalCompleted,
    std::atomic<uint64_t>& globalErrors,
    std::vector<double>& latenciesMicroseconds,
    std::mutex& latencyMutex) {

    try {
        boost::asio::io_context ioContext;
        tcp::resolver resolver(ioContext);
        auto endpoints = resolver.resolve(host, std::to_string(port));

        tcp::socket socket(ioContext);
        boost::asio::connect(socket, endpoints);

        std::string testPayload = "AsyncNet High-Performance Benchmark Payload Engine 2026!";
        asyncnet::Packet pkt;
        pkt.header.magic = asyncnet::MAGIC_HEADER;
        pkt.header.type = asyncnet::MessageType::ECHO_REQUEST;
        pkt.header.length = static_cast<uint32_t>(testPayload.size());
        pkt.payload.assign(testPayload.begin(), testPayload.end());

        auto reqData = asyncnet::Protocol::serializePacket(pkt);
        std::vector<double> threadLatencies;
        threadLatencies.reserve(requestsPerClient);

        std::vector<uint8_t> headerBuf(asyncnet::HEADER_SIZE);
        std::vector<uint8_t> payloadBuf;

        for (int i = 0; i < requestsPerClient; ++i) {
            auto start = std::chrono::high_resolution_clock::now();

            // 1. Send Echo Request
            boost::asio::write(socket, boost::asio::buffer(reqData));

            // 2. Read Response Header
            boost::asio::read(socket, boost::asio::buffer(headerBuf, asyncnet::HEADER_SIZE));
            asyncnet::Header respHeader;
            if (!asyncnet::Protocol::deserializeHeader(headerBuf.data(), asyncnet::HEADER_SIZE, respHeader)) {
                globalErrors++;
                continue;
            }

            // 3. Read Payload
            if (respHeader.length > 0) {
                payloadBuf.resize(respHeader.length);
                boost::asio::read(socket, boost::asio::buffer(payloadBuf, respHeader.length));
            }

            auto end = std::chrono::high_resolution_clock::now();
            double latencyUs = std::chrono::duration<double, std::micro>(end - start).count();
            threadLatencies.push_back(latencyUs);
            globalCompleted++;
        }

        // Send Disconnect message
        asyncnet::Packet discPkt;
        discPkt.header.magic = asyncnet::MAGIC_HEADER;
        discPkt.header.type = asyncnet::MessageType::DISCONNECT;
        discPkt.header.length = 0;
        auto discData = asyncnet::Protocol::serializePacket(discPkt);
        boost::asio::write(socket, boost::asio::buffer(discData));

        boost::system::error_code ec;
        socket.shutdown(tcp::socket::shutdown_both, ec);
        socket.close(ec);

        {
            std::lock_guard<std::mutex> lock(latencyMutex);
            latenciesMicroseconds.insert(latenciesMicroseconds.end(), threadLatencies.begin(), threadLatencies.end());
        }

    } catch (const std::exception& e) {
        globalErrors++;
    }
}

int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    uint16_t port = 8080;
    int numClients = 20;
    int requestsPerClient = 1000;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--host" && i + 1 < argc) host = argv[++i];
        else if (arg == "--port" && i + 1 < argc) port = static_cast<uint16_t>(std::atoi(argv[++i]));
        else if (arg == "--clients" && i + 1 < argc) numClients = std::atoi(argv[++i]);
        else if (arg == "--requests" && i + 1 < argc) requestsPerClient = std::atoi(argv[++i]);
    }

    std::cout << "\n======================================================\n"
              << "  AsyncNet-Server Benchmark Load Generator\n"
              << "======================================================\n"
              << "Target Host:          " << host << ":" << port << "\n"
              << "Concurrent Clients:   " << numClients << "\n"
              << "Requests / Client:    " << requestsPerClient << "\n"
              << "Total Workload:       " << (numClients * requestsPerClient) << " requests\n"
              << "------------------------------------------------------\n"
              << "Running benchmark...\n";

    std::atomic<uint64_t> completedRequests{0};
    std::atomic<uint64_t> errorCount{0};
    std::vector<double> latenciesUs;
    std::mutex latencyMutex;

    auto startTime = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    threads.reserve(numClients);

    for (int i = 0; i < numClients; ++i) {
        threads.emplace_back(
            runClientWorker,
            std::ref(host),
            port,
            requestsPerClient,
            std::ref(completedRequests),
            std::ref(errorCount),
            std::ref(latenciesUs),
            std::ref(latencyMutex)
        );
    }

    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double totalDurationSec = std::chrono::duration<double>(endTime - startTime).count();

    double totalLatencyUs = std::accumulate(latenciesUs.begin(), latenciesUs.end(), 0.0);
    double avgLatencyUs = latenciesUs.empty() ? 0.0 : totalLatencyUs / latenciesUs.size();

    // Sort to compute latency percentiles
    std::sort(latenciesUs.begin(), latenciesUs.end());
    double p50 = latenciesUs.empty() ? 0.0 : latenciesUs[latenciesUs.size() * 0.50];
    double p95 = latenciesUs.empty() ? 0.0 : latenciesUs[latenciesUs.size() * 0.95];
    double p99 = latenciesUs.empty() ? 0.0 : latenciesUs[latenciesUs.size() * 0.99];

    double reqPerSec = completedRequests / totalDurationSec;

    std::cout << "\n======================================================\n"
              << "                BENCHMARK RESULTS                     \n"
              << "======================================================\n"
              << "Total Duration:       " << std::fixed << std::setprecision(3) << totalDurationSec << " seconds\n"
              << "Successful Requests:  " << completedRequests << "\n"
              << "Failed Requests:      " << errorCount << "\n"
              << "Throughput:           " << std::setprecision(2) << reqPerSec << " req/sec\n"
              << "------------------------------------------------------\n"
              << "Latency Profile:\n"
              << "  Average (Mean):     " << std::setprecision(2) << avgLatencyUs << " us\n"
              << "  50th %ile (p50):    " << std::setprecision(2) << p50 << " us\n"
              << "  95th %ile (p95):    " << std::setprecision(2) << p95 << " us\n"
              << "  99th %ile (p99):    " << std::setprecision(2) << p99 << " us\n"
              << "======================================================\n\n";

    return 0;
}
