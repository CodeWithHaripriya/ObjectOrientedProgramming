#include "Server.hpp"
#include "Logger.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <cstdlib>

void printUsage(const char* progName) {
    std::cout << "Usage: " << progName << " [options]\n"
              << "Options:\n"
              << "  -p, --port <port>       Port number to listen on (default: 8080)\n"
              << "  -t, --threads <count>   Number of worker threads (default: hardware concurrency)\n"
              << "  -h, --help              Show this help message\n";
}

int main(int argc, char* argv[]) {
    uint16_t port = 8080;
    size_t threads = std::thread::hardware_concurrency();
    if (threads == 0) threads = 4;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-p" || arg == "--port") {
            if (i + 1 < argc) {
                port = static_cast<uint16_t>(std::atoi(argv[++i]));
            }
        } else if (arg == "-t" || arg == "--threads") {
            if (i + 1 < argc) {
                threads = static_cast<size_t>(std::atoi(argv[++i]));
            }
        } else if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        }
    }

    try {
        asyncnet::Server server(port, threads);
        server.start();

        // Keep main thread alive while server runs
        std::cout << "\n======================================================\n"
                  << "  AsyncNet-Server is running on port " << port << "\n"
                  << "  Press Ctrl+C to terminate cleanly.\n"
                  << "======================================================\n\n";

        // Wait until server stops (triggered by SIGINT/SIGTERM)
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

    } catch (const std::exception& e) {
        LOG_ERROR("Fatal error starting server: " + std::string(e.what()));
        return 1;
    }

    return 0;
}
