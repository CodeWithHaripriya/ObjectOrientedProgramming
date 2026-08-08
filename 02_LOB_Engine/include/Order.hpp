#ifndef LOB_ORDER_HPP
#define LOB_ORDER_HPP

#include "Types.hpp"

namespace lob {

struct PriceLevel; // Forward declaration

struct Order {
    OrderId id{0};
    Price price{0};
    Quantity quantity{0};
    Quantity remainingQty{0};
    Side side{Side::BUY};
    OrderType type{OrderType::LIMIT};
    Timestamp timestamp{0};
    PriceLevel* priceLevelPtr{nullptr};

    // Intrusive Doubly-Linked List Pointers for O(1) unlinking/cancellation
    Order* prev{nullptr};
    Order* next{nullptr};

    Order() = default;

    Order(OrderId orderId, Price p, Quantity q, Side s, OrderType t, Timestamp ts = 0)
        : id(orderId), price(p), quantity(q), remainingQty(q), side(s), type(t), timestamp(ts) {}
};

} // namespace lob

#endif // LOB_ORDER_HPP
