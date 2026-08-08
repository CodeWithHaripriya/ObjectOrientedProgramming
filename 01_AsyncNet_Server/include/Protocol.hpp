#ifndef ASYNCNET_PROTOCOL_HPP
#define ASYNCNET_PROTOCOL_HPP

#include "Common.hpp"
#include <vector>
#include <cstdint>

namespace asyncnet {

class Protocol {
public:
    // Serializes header into network byte order (Big Endian)
    static std::vector<uint8_t> serializeHeader(const Header& header);
    
    // Deserializes buffer into Header struct. Returns true on success.
    static bool deserializeHeader(const uint8_t* data, size_t size, Header& header);

    // Serializes full Packet (Header + Payload) into byte buffer
    static std::vector<uint8_t> serializePacket(const Packet& packet);

    // Creates an Echo Response packet from payload
    static Packet createEchoResponse(const std::vector<uint8_t>& payload);

    // Creates a Pong packet
    static Packet createPongPacket();
};

} // namespace asyncnet

#endif // ASYNCNET_PROTOCOL_HPP
