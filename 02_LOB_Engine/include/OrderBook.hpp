#ifndef LOB_ORDER_BOOK_HPP
#define LOB_ORDER_BOOK_HPP

#include "Types.hpp"
#include "Order.hpp"
#include "PriceLevel.hpp"
#include "ObjectPool.hpp"
#include <map>
#include <unordered_map>
#include <vector>
#include <span>

namespace lob {

constexpr size_t DEFAULT_MAX_ORDERS = 100000;
constexpr size_t DEFAULT_MAX_LEVELS = 10000;

class OrderBook {
public:
    OrderBook();
    ~OrderBook();

    OrderBook(const OrderBook&) = delete;
    OrderBook& operator=(const OrderBook&) = delete;

    // Submits a new order and matches against opposite book side
    std::vector<MatchEvent> addOrder(OrderId id, Side side, Price price, Quantity qty, OrderType type = OrderType::LIMIT);

    // Cancels an active order in O(1) time
    bool cancelOrder(OrderId id);

    // Query Top-of-Book
    bool getBestBid(Price& price, Quantity& qty) const;
    bool getBestAsk(Price& price, Quantity& qty) const;
    bool getSpread(Price& spread) const;

    size_t getBidLevelsCount() const noexcept { return bids_.size(); }
    size_t getAskLevelsCount() const noexcept { return asks_.size(); }
    size_t getActiveOrdersCount() const noexcept { return orderMap_.size(); }

    uint64_t getTotalTradesCount() const noexcept { return totalTradesCount_; }
    uint64_t getTotalVolumeTraded() const noexcept { return totalVolumeTraded_; }

    void printBook(size_t maxLevels = 5) const;

private:
    void matchBuyOrder(Order* buyOrder, std::vector<MatchEvent>& matches);
    void matchSellOrder(Order* sellOrder, std::vector<MatchEvent>& matches);
    
    PriceLevel* getOrCreatePriceLevel(Side side, Price price);
    void removePriceLevelIfEmpty(Side side, Price price, PriceLevel* level);

    // Bids stored in descending order (highest price first)
    std::map<Price, PriceLevel*, std::greater<Price>> bids_;

    // Asks stored in ascending order (lowest price first)
    std::map<Price, PriceLevel*, std::less<Price>> asks_;

    // O(1) Order lookup map
    std::unordered_map<OrderId, Order*> orderMap_;

    // Pre-allocated Object Pools for zero dynamic memory allocation
    ObjectPool<Order, DEFAULT_MAX_ORDERS> orderPool_;
    ObjectPool<PriceLevel, DEFAULT_MAX_LEVELS> levelPool_;

    uint64_t totalTradesCount_{0};
    uint64_t totalVolumeTraded_{0};
};

} // namespace lob

#endif // LOB_ORDER_BOOK_HPP
