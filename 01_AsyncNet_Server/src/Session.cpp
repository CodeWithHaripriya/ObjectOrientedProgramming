#include "Session.hpp"
#include "SessionManager.hpp"
#include "Logger.hpp"
#include <iostream>

namespace asyncnet {

Session::Session(boost::asio::ip::tcp::socket socket, SessionManager& manager)
    : socket_(std::move(socket)),
      strand_(boost::asio::make_strand(socket_.get_executor())),
      manager_(manager),
      headerBuffer_(HEADER_SIZE) {
    try {
        auto endpoint = socket_.remote_endpoint();
        remoteAddress_ = endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
    } catch (const std::exception& e) {
        remoteAddress_ = "unknown";
    }
}

Session::~Session() {
    LOG_DEBUG("Session destroyed for " + remoteAddress_);
}

void Session::start() {
    LOG_INFO("Client connected from " + remoteAddress_);
    doReadHeader();
}

void Session::stop() {
    auto self(shared_from_this());
    boost::asio::post(strand_, [this, self]() {
        if (isStopped_) return;
        isStopped_ = true;

        boost::system::error_code ec;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        socket_.close(ec);

        LOG_INFO("Client disconnected: " + remoteAddress_);
        manager_.removeSession(shared_from_this());
    });
}

std::string Session::getRemoteAddress() const {
    return remoteAddress_;
}

void Session::send(const Packet& packet) {
    auto data = Protocol::serializePacket(packet);
    auto self(shared_from_this());

    boost::asio::post(strand_, [this, self, data = std::move(data)]() mutable {
        if (isStopped_) return;

        bool writeInProgress = !writeQueue_.empty();
        writeQueue_.push_back(std::move(data));
        
        if (!writeInProgress) {
            doWrite();
        }
    });
}

void Session::doReadHeader() {
    auto self(shared_from_this());

    boost::asio::async_read(
        socket_,
        boost::asio::buffer(headerBuffer_, HEADER_SIZE),
        boost::asio::bind_executor(
            strand_,
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (ec) {
                    if (ec != boost::asio::error::eof && ec != boost::asio::error::operation_aborted) {
                        LOG_WARN("Read header error from " + remoteAddress_ + ": " + ec.message());
                    }
                    stop();
                    return;
                }

                Header header;
                if (!Protocol::deserializeHeader(headerBuffer_.data(), length, header)) {
                    LOG_WARN("Malformed magic bytes or header from " + remoteAddress_);
                    stop();
                    return;
                }

                if (header.length > MAX_PAYLOAD_SIZE) {
                    LOG_WARN("Payload size exceeds maximum allowed limit from " + remoteAddress_);
                    stop();
                    return;
                }

                if (header.length > 0) {
                    doReadPayload(header);
                } else {
                    processPacket(header, {});
                    doReadHeader();
                }
            }));
}

void Session::doReadPayload(Header header) {
    payloadBuffer_.resize(header.length);
    auto self(shared_from_this());

    boost::asio::async_read(
        socket_,
        boost::asio::buffer(payloadBuffer_, header.length),
        boost::asio::bind_executor(
            strand_,
            [this, self, header](boost::system::error_code ec, std::size_t length) {
                if (ec) {
                    LOG_WARN("Read payload error from " + remoteAddress_ + ": " + ec.message());
                    stop();
                    return;
                }

                processPacket(header, payloadBuffer_);
                doReadHeader();
            }));
}

void Session::processPacket(const Header& header, const std::vector<uint8_t>& payload) {
    manager_.incrementProcessedMessages();

    switch (header.type) {
        case MessageType::PING: {
            LOG_DEBUG("Received PING from " + remoteAddress_);
            send(Protocol::createPongPacket());
            break;
        }
        case MessageType::ECHO_REQUEST: {
            LOG_DEBUG("Received ECHO_REQUEST (" + std::to_string(payload.size()) + " bytes) from " + remoteAddress_);
            send(Protocol::createEchoResponse(payload));
            break;
        }
        case MessageType::DISCONNECT: {
            LOG_INFO("Client requested disconnect: " + remoteAddress_);
            stop();
            break;
        }
        default: {
            LOG_WARN("Unknown message type received from " + remoteAddress_);
            break;
        }
    }
}

void Session::doWrite() {
    auto self(shared_from_this());

    boost::asio::async_write(
        socket_,
        boost::asio::buffer(writeQueue_.front()),
        boost::asio::bind_executor(
            strand_,
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (ec) {
                    LOG_WARN("Write error to " + remoteAddress_ + ": " + ec.message());
                    stop();
                    return;
                }

                writeQueue_.pop_front();
                if (!writeQueue_.empty()) {
                    doWrite();
                }
            }));
}

} // namespace asyncnet
