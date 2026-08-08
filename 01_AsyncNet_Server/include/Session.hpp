#ifndef ASYNCNET_SESSION_HPP
#define ASYNCNET_SESSION_HPP

#include "Common.hpp"
#include "Protocol.hpp"
#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include <deque>
#include <mutex>

namespace asyncnet {

class SessionManager;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(boost::asio::ip::tcp::socket socket, SessionManager& manager);
    ~Session();

    void start();
    void stop();
    void send(const Packet& packet);
    
    std::string getRemoteAddress() const;

private:
    void doReadHeader();
    void doReadPayload(Header header);
    void processPacket(const Header& header, const std::vector<uint8_t>& payload);
    void doWrite();

    boost::asio::ip::tcp::socket socket_;
    boost::asio::strand<boost::asio::any_io_executor> strand_;
    SessionManager& manager_;

    std::vector<uint8_t> headerBuffer_;
    std::vector<uint8_t> payloadBuffer_;
    
    std::deque<std::vector<uint8_t>> writeQueue_;
    bool isWriting_{false};
    bool isStopped_{false};
    std::string remoteAddress_;
};

} // namespace asyncnet

#endif // ASYNCNET_SESSION_HPP
