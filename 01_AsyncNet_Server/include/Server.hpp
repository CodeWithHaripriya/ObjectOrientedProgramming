#ifndef ASYNCNET_SERVER_HPP
#define ASYNCNET_SERVER_HPP

#include "Common.hpp"
#include "SessionManager.hpp"
#include <boost/asio.hpp>
#include <vector>
#include <thread>
#include <memory>
#include <atomic>

namespace asyncnet {

class Server {
public:
    Server(uint16_t port, size_t threadPoolSize);
    ~Server();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    void start();
    void stop();

    uint16_t getPort() const { return port_; }
    size_t getThreadPoolSize() const { return threadPoolSize_; }
    const SessionManager& getSessionManager() const { return sessionManager_; }

private:
    void doAccept();

    uint16_t port_;
    size_t threadPoolSize_;
    
    boost::asio::io_context ioContext_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::signal_set signals_;
    
    SessionManager sessionManager_;
    std::vector<std::thread> workerThreads_;
    std::atomic<bool> isRunning_{false};
};

} // namespace asyncnet

#endif // ASYNCNET_SERVER_HPP
