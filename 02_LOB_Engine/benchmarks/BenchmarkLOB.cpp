#include "OrderBook.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include <random>

int main() {
    constexpr size_t NUM_OPERATIONS = 500000;

    std::cout << "\n======================================================\n";
    std::cout << "  LOB-Engine High-Frequency Latency Benchmark\n";
    std::cout << "======================================================\n";
    std::cout << "Workload: " << NUM_OPERATIONS << " order operations\n";
    std::cout << "Benchmarking hot path execution latency...\n\n";

    lob::OrderBook book;

    std::vector<double> latenciesNs;
    latenciesNs.reserve(NUM_OPERATIONS);

    std::mt19937 rng(42);
    std::uniform_int_distribution<uint64_t> priceDist(90, 110);
    std::uniform_int_distribution<uint32_t> qtyDist(1, 100);
    std::uniform_int_distribution<int> sideDist(0, 1);

    auto startTime = std::chrono::high_resolution_clock::now();

    for (size_t i = 1; i <= NUM_OPERATIONS; ++i) {
        lob::Side side = (sideDist(rng) == 0) ? lob::Side::BUY : lob::Side::SELL;
        lob::Price price = priceDist(rng);
        lob::Quantity qty = qtyDist(rng);

        auto opStart = std::chrono::high_resolution_clock::now();

        // 80% Add Orders, 20% Cancel Orders
        if (i % 5 == 0 && i > 10) {
            book.cancelOrder(i - 5);
        } else {
            book.addOrder(i, side, price, qty);
        }

        auto opEnd = std::chrono::high_resolution_clock::now();
        double ns = std::chrono::duration<double, std::nano>(opEnd - opStart).count();
        latenciesNs.push_back(ns);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double totalSec = std::chrono::duration<double>(endTime - startTime).count();

    std::sort(latenciesNs.begin(), latenciesNs.end());

    double sum = std::accumulate(latenciesNs.begin(), latenciesNs.end(), 0.0);
    double meanNs = sum / latenciesNs.size();
    double p50 = latenciesNs[latenciesNs.size() * 0.50];
    double p90 = latenciesNs[latenciesNs.size() * 0.90];
    double p99 = latenciesNs[latenciesNs.size() * 0.99];
    double p999 = latenciesNs[latenciesNs.size() * 0.999];

    double opsPerSec = NUM_OPERATIONS / totalSec;

    std::cout << "======================================================\n";
    std::cout << "                BENCHMARK RESULTS                     \n";
    std::cout << "======================================================\n";
    std::cout << "Total Time Elapsed:   " << std::fixed << std::setprecision(4) << totalSec << " seconds\n";
    std::cout << "Throughput:           " << std::setprecision(2) << opsPerSec << " ops/sec\n";
    std::cout << "------------------------------------------------------\n";
    std::cout << "Latency Distribution Profile:\n";
    std::cout << "  Mean Latency:       " << std::setprecision(2) << meanNs << " ns (" << (meanNs / 1000.0) << " us)\n";
    std::cout << "  50th %ile (p50):    " << std::setprecision(2) << p50 << " ns (" << (p50 / 1000.0) << " us)\n";
    std::cout << "  90th %ile (p90):    " << std::setprecision(2) << p90 << " ns (" << (p90 / 1000.0) << " us)\n";
    std::cout << "  99th %ile (p99):    " << std::setprecision(2) << p99 << " ns (" << (p99 / 1000.0) << " us)\n";
    std::cout << "  99.9th %ile (p99.9): " << std::setprecision(2) << p999 << " ns (" << (p999 / 1000.0) << " us)\n";
    std::cout << "======================================================\n\n";

    return 0;
}
