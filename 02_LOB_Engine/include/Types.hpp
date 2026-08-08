#ifndef LOB_TYPES_HPP
#define LOB_TYPES_HPP

#include <cstdint>
#include <concepts>
#include <iostream>
#include <string_view>

namespace lob {

using OrderId = uint64_t;
using Price = uint64_t;   // Represented in ticks/cents to avoid floating point imprecision
using Quantity = uint32_t;
using Timestamp = uint64_t; // Nanoseconds since epoch

enum class Side : uint8_t {
    BUY,
    SELL
};

enum class OrderType : uint8_t {
    LIMIT,
    MARKET,
    CANCEL
};

enum class Status : uint8_t {
    NEW,
    PARTIALLY_FILLED,
    FILLED,
    CANCELED,
    REJECTED
};

struct MatchEvent {
    OrderId makerOrderId;
    OrderId takerOrderId;
    Price matchPrice;
    Quantity matchQuantity;
    Timestamp timestamp;
};

// C++20 Concepts for Compile-Time Type Safety
template <typename T>
concept PriceConcept = std::integral<T> && sizeof(T) >= 4;

template <typename T>
concept QuantityConcept = std::integral<T> && sizeof(T) >= 4;

inline std::string_view sideToString(Side side) {
    return (side == Side::BUY) ? "BUY" : "SELL";
}

} // namespace lob

#endif // LOB_TYPES_HPP
