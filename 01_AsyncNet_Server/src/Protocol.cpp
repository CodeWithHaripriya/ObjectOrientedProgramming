#include "Protocol.hpp"
#include <arpa/inet.h>
#include <cstring>

namespace asyncnet {

std::vector<uint8_t> Protocol::serializeHeader(const Header& header) {
    std::vector<uint8_t> buffer(HEADER_SIZE);
    
    uint16_t netMagic = htons(header.magic);
    uint16_t netType = htons(static_cast<uint16_t>(header.type));
    uint32_t netLen = htonl(header.length);

    std::memcpy(buffer.data(), &netMagic, sizeof(netMagic));
    std::memcpy(buffer.data() + 2, &netType, sizeof(netType));
    std::memcpy(buffer.data() + 4, &netLen, sizeof(netLen));

    return buffer;
}

bool Protocol::deserializeHeader(const uint8_t* data, size_t size, Header& header) {
    if (size < HEADER_SIZE) {
        return false;
    }

    uint16_t netMagic = 0;
    uint16_t netType = 0;
    uint32_t netLen = 0;

    std::memcpy(&netMagic, data, sizeof(netMagic));
    std::memcpy(&netType, data + 2, sizeof(netType));
    std::memcpy(&netLen, data + 4, sizeof(netLen));

    header.magic = ntohs(netMagic);
    header.type = static_cast<MessageType>(ntohs(netType));
    header.length = ntohl(netLen);

    if (header.magic != MAGIC_HEADER) {
        return false;
    }

    return true;
}

std::vector<uint8_t> Protocol::serializePacket(const Packet& packet) {
    Header h = packet.header;
    h.length = static_cast<uint32_t>(packet.payload.size());

    std::vector<uint8_t> buffer = serializeHeader(h);
    buffer.insert(buffer.end(), packet.payload.begin(), packet.payload.end());

    return buffer;
}

Packet Protocol::createEchoResponse(const std::vector<uint8_t>& payload) {
    Packet pkt;
    pkt.header.magic = MAGIC_HEADER;
    pkt.header.type = MessageType::ECHO_RESPONSE;
    pkt.header.length = static_cast<uint32_t>(payload.size());
    pkt.payload = payload;
    return pkt;
}

Packet Protocol::createPongPacket() {
    Packet pkt;
    pkt.header.magic = MAGIC_HEADER;
    pkt.header.type = MessageType::PONG;
    pkt.header.length = 0;
    return pkt;
}

} // namespace asyncnet
