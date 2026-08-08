#ifndef ASYNCNET_COMMON_HPP
#define ASYNCNET_COMMON_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <iostream>

namespace asyncnet {

constexpr uint16_t MAGIC_HEADER = 0x414E; // 'AN' in hex (AsyncNet)
constexpr size_t HEADER_SIZE = 8;         // Magic(2B) + Type(2B) + Length(4B)
constexpr size_t MAX_PAYLOAD_SIZE = 65536; // 64 KB limit per frame

enum class MessageType : uint16_t {
    PING = 0x0001,
    PONG = 0x0002,
    ECHO_REQUEST = 0x0003,
    ECHO_RESPONSE = 0x0004,
    DATA_STREAM = 0x0005,
    DISCONNECT = 0x00FF,
    UNKNOWN = 0xFFFF
};

struct Header {
    uint16_t magic{MAGIC_HEADER};
    MessageType type{MessageType::UNKNOWN};
    uint32_t length{0};
};

struct Packet {
    Header header;
    std::vector<uint8_t> payload;
};

} // namespace asyncnet

#endif // ASYNCNET_COMMON_HPP
