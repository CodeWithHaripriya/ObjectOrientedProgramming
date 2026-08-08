#ifndef LOB_PRICE_LEVEL_HPP
#define LOB_PRICE_LEVEL_HPP

#include "Order.hpp"

namespace lob {

struct PriceLevel {
    Price price{0};
    Quantity totalVolume{0};
    size_t orderCount{0};

    Order* head{nullptr};
    Order* tail{nullptr};

    PriceLevel* prevLevel{nullptr};
    PriceLevel* nextLevel{nullptr};

    explicit PriceLevel(Price p = 0) : price(p) {}

    // O(1) Push to Tail (FIFO priority)
    void append(Order* order) {
        order->priceLevelPtr = this;
        order->next = nullptr;
        order->prev = tail;

        if (tail) {
            tail->next = order;
        } else {
            head = order;
        }
        tail = order;

        totalVolume += order->remainingQty;
        ++orderCount;
    }

    // O(1) Intrusive Unlink
    void remove(Order* order) {
        if (!order) return;

        if (order->prev) {
            order->prev->next = order->next;
        } else {
            head = order->next;
        }

        if (order->next) {
            order->next->prev = order->prev;
        } else {
            tail = order->prev;
        }

        totalVolume -= (totalVolume >= order->remainingQty) ? order->remainingQty : totalVolume;
        if (orderCount > 0) --orderCount;

        order->next = nullptr;
        order->prev = nullptr;
        order->priceLevelPtr = nullptr;
    }

    bool empty() const noexcept {
        return head == nullptr;
    }
};

} // namespace lob

#endif // LOB_PRICE_LEVEL_HPP
