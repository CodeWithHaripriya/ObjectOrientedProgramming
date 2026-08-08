#include "Protocol.hpp"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>

void testHeaderSerialization() {
    asyncnet::Header original;
    original.magic = asyncnet::MAGIC_HEADER;
    original.type = asyncnet::MessageType::ECHO_REQUEST;
    original.length = 1024;

    auto bytes = asyncnet::Protocol::serializeHeader(original);
    assert(bytes.size() == asyncnet::HEADER_SIZE);

    asyncnet::Header deserialized;
    bool success = asyncnet::Protocol::deserializeHeader(bytes.data(), bytes.size(), deserialized);
    (void)success;
    assert(success);
    assert(deserialized.magic == original.magic);
    assert(deserialized.type == original.type);
    assert(deserialized.length == original.length);

    std::cout << "[PASS] testHeaderSerialization\n";
}

void testInvalidMagicHeader() {
    asyncnet::Header original;
    original.magic = 0xDEAD; // Invalid magic
    original.type = asyncnet::MessageType::PING;
    original.length = 0;

    auto bytes = asyncnet::Protocol::serializeHeader(original);

    asyncnet::Header deserialized;
    bool success = asyncnet::Protocol::deserializeHeader(bytes.data(), bytes.size(), deserialized);
    (void)success;
    assert(!success); // Must fail due to magic header mismatch
    std::cout << "[PASS] testInvalidMagicHeader\n";
}

void testPacketSerialization() {
    asyncnet::Packet originalPkt;
    originalPkt.header.magic = asyncnet::MAGIC_HEADER;
    originalPkt.header.type = asyncnet::MessageType::ECHO_REQUEST;
    std::string text = "Hello AsyncNet Protocol";
    originalPkt.payload.assign(text.begin(), text.end());
    originalPkt.header.length = static_cast<uint32_t>(originalPkt.payload.size());

    auto packetBytes = asyncnet::Protocol::serializePacket(originalPkt);
    assert(packetBytes.size() == asyncnet::HEADER_SIZE + text.size());

    asyncnet::Header parsedHeader;
    bool success = asyncnet::Protocol::deserializeHeader(packetBytes.data(), packetBytes.size(), parsedHeader);
    (void)success;
    assert(success);
    assert(parsedHeader.length == text.size());
    assert(parsedHeader.type == asyncnet::MessageType::ECHO_REQUEST);

    std::string payloadStr(packetBytes.begin() + asyncnet::HEADER_SIZE, packetBytes.end());
    assert(payloadStr == text);

    std::cout << "[PASS] testPacketSerialization\n";
}

int main() {
    std::cout << "Running AsyncNet Protocol Unit Tests...\n";
    testHeaderSerialization();
    testInvalidMagicHeader();
    testPacketSerialization();
    std::cout << "ALL PROTOCOL UNIT TESTS PASSED SUCCESSFULLY!\n";
    return 0;
}
