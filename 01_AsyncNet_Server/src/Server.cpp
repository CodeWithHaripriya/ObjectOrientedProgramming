#include "Server.hpp"
#include "Session.hpp"
#include "Logger.hpp"

namespace asyncnet {

Server::Server(uint16_t port, size_t threadPoolSize)
    : port_(port),
      threadPoolSize_(threadPoolSize),
      acceptor_(ioContext_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
      signals_(ioContext_, SIGINT, SIGTERM) {
    
    signals_.async_wait([this](boost::system::error_code ec, int signal) {
        if (!ec) {
            LOG_INFO("Signal received (" + std::to_string(signal) + "). Initiating graceful shutdown...");
            stop();
        }
    });
}

Server::~Server() {
    stop();
}

void Server::start() {
    if (isRunning_) return;
    isRunning_ = true;

    LOG_INFO("Starting AsyncNet-Server on port " + std::to_string(port_) +
             " with " + std::to_string(threadPoolSize_) + " worker threads.");

    doAccept();

    workerThreads_.reserve(threadPoolSize_);
    for (size_t i = 0; i < threadPoolSize_; ++i) {
        workerThreads_.emplace_back([this, i]() {
            LOG_DEBUG("Worker thread " + std::to_string(i + 1) + " started.");
            try {
                ioContext_.run();
            } catch (const std::exception& e) {
                LOG_ERROR("Exception in worker thread: " + std::string(e.what()));
            }
            LOG_DEBUG("Worker thread " + std::to_string(i + 1) + " stopped.");
        });
    }
}

void Server::stop() {
    if (!isRunning_) return;
    isRunning_ = false;

    LOG_INFO("Shutting down AsyncNet-Server...");

    sessionManager_.stopAll();
    acceptor_.close();
    ioContext_.stop();

    for (auto& thread : workerThreads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    workerThreads_.clear();

    LOG_INFO("AsyncNet-Server successfully stopped.");
}

void Server::doAccept() {
    acceptor_.async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
        if (!ec) {
            auto session = std::make_shared<Session>(std::move(socket), sessionManager_);
            sessionManager_.addSession(session);
            session->start();
        } else {
            if (ec != boost::asio::error::operation_aborted) {
                LOG_ERROR("Accept failed: " + ec.message());
            }
        }

        if (isRunning_) {
            doAccept();
        }
    });
}

} // namespace asyncnet
