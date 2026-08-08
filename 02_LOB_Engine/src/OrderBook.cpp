#include "OrderBook.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>

namespace lob {

OrderBook::OrderBook() = default;

OrderBook::~OrderBook() {
    // Return all orders to pool
    for (auto& [id, order] : orderMap_) {
        orderPool_.deallocate(order);
    }
    orderMap_.clear();

    for (auto& [price, level] : bids_) {
        levelPool_.deallocate(level);
    }
    bids_.clear();

    for (auto& [price, level] : asks_) {
        levelPool_.deallocate(level);
    }
    asks_.clear();
}

std::vector<MatchEvent> OrderBook::addOrder(OrderId id, Side side, Price price, Quantity qty, OrderType type) {
    std::vector<MatchEvent> matches;

    if (qty == 0 || orderMap_.find(id) != orderMap_.end()) {
        return matches; // Invalid order or duplicate ID
    }

    Order* incomingOrder = orderPool_.allocate(id, price, qty, side, type, 0);

    if (side == Side::BUY) {
        matchBuyOrder(incomingOrder, matches);
        if (incomingOrder->remainingQty > 0 && type == OrderType::LIMIT) {
            PriceLevel* level = getOrCreatePriceLevel(Side::BUY, incomingOrder->price);
            level->append(incomingOrder);
            orderMap_[id] = incomingOrder;
        } else {
            orderPool_.deallocate(incomingOrder);
        }
    } else { // Side::SELL
        matchSellOrder(incomingOrder, matches);
        if (incomingOrder->remainingQty > 0 && type == OrderType::LIMIT) {
            PriceLevel* level = getOrCreatePriceLevel(Side::SELL, incomingOrder->price);
            level->append(incomingOrder);
            orderMap_[id] = incomingOrder;
        } else {
            orderPool_.deallocate(incomingOrder);
        }
    }

    return matches;
}

void OrderBook::matchBuyOrder(Order* buyOrder, std::vector<MatchEvent>& matches) {
    auto askIt = asks_.begin();

    while (askIt != asks_.end() && buyOrder->remainingQty > 0) {
        PriceLevel* askLevel = askIt->second;

        // For LIMIT orders, check if Ask price exceeds Buy limit price
        if (buyOrder->type == OrderType::LIMIT && askLevel->price > buyOrder->price) {
            break;
        }

        Order* makerOrder = askLevel->head;
        while (makerOrder && buyOrder->remainingQty > 0) {
            Order* nextMaker = makerOrder->next;

            Quantity fillQty = std::min(buyOrder->remainingQty, makerOrder->remainingQty);

            buyOrder->remainingQty -= fillQty;
            makerOrder->remainingQty -= fillQty;
            askLevel->totalVolume -= fillQty;

            MatchEvent match{
                .makerOrderId = makerOrder->id,
                .takerOrderId = buyOrder->id,
                .matchPrice = askLevel->price,
                .matchQuantity = fillQty,
                .timestamp = static_cast<Timestamp>(
                    std::chrono::high_resolution_clock::now().time_since_epoch().count())
            };

            matches.push_back(match);
            totalTradesCount_++;
            totalVolumeTraded_ += fillQty;

            if (makerOrder->remainingQty == 0) {
                orderMap_.erase(makerOrder->id);
                askLevel->remove(makerOrder);
                orderPool_.deallocate(makerOrder);
            }

            makerOrder = nextMaker;
        }

        if (askLevel->empty()) {
            levelPool_.deallocate(askLevel);
            askIt = asks_.erase(askIt);
        } else {
            ++askIt;
        }
    }
}

void OrderBook::matchSellOrder(Order* sellOrder, std::vector<MatchEvent>& matches) {
    auto bidIt = bids_.begin();

    while (bidIt != bids_.end() && sellOrder->remainingQty > 0) {
        PriceLevel* bidLevel = bidIt->second;

        // For LIMIT orders, check if Bid price is lower than Sell limit price
        if (sellOrder->type == OrderType::LIMIT && bidLevel->price < sellOrder->price) {
            break;
        }

        Order* makerOrder = bidLevel->head;
        while (makerOrder && sellOrder->remainingQty > 0) {
            Order* nextMaker = makerOrder->next;

            Quantity fillQty = std::min(sellOrder->remainingQty, makerOrder->remainingQty);

            sellOrder->remainingQty -= fillQty;
            makerOrder->remainingQty -= fillQty;
            bidLevel->totalVolume -= fillQty;

            MatchEvent match{
                .makerOrderId = makerOrder->id,
                .takerOrderId = sellOrder->id,
                .matchPrice = bidLevel->price,
                .matchQuantity = fillQty,
                .timestamp = static_cast<Timestamp>(
                    std::chrono::high_resolution_clock::now().time_since_epoch().count())
            };

            matches.push_back(match);
            totalTradesCount_++;
            totalVolumeTraded_ += fillQty;

            if (makerOrder->remainingQty == 0) {
                orderMap_.erase(makerOrder->id);
                bidLevel->remove(makerOrder);
                orderPool_.deallocate(makerOrder);
            }

            makerOrder = nextMaker;
        }

        if (bidLevel->empty()) {
            levelPool_.deallocate(bidLevel);
            bidIt = bids_.erase(bidIt);
        } else {
            ++bidIt;
        }
    }
}

bool OrderBook::cancelOrder(OrderId id) {
    auto it = orderMap_.find(id);
    if (it == orderMap_.end()) {
        return false;
    }

    Order* order = it->second;
    PriceLevel* level = order->priceLevelPtr;

    if (level) {
        level->remove(order);
        removePriceLevelIfEmpty(order->side, order->price, level);
    }

    orderMap_.erase(it);
    orderPool_.deallocate(order);
    return true;
}

bool OrderBook::getBestBid(Price& price, Quantity& qty) const {
    if (bids_.empty()) return false;
    price = bids_.begin()->first;
    qty = bids_.begin()->second->totalVolume;
    return true;
}

bool OrderBook::getBestAsk(Price& price, Quantity& qty) const {
    if (asks_.empty()) return false;
    price = asks_.begin()->first;
    qty = asks_.begin()->second->totalVolume;
    return true;
}

bool OrderBook::getSpread(Price& spread) const {
    if (bids_.empty() || asks_.empty()) return false;
    Price bestBid = bids_.begin()->first;
    Price bestAsk = asks_.begin()->first;

    if (bestAsk >= bestBid) {
        spread = bestAsk - bestBid;
        return true;
    }
    return false;
}

PriceLevel* OrderBook::getOrCreatePriceLevel(Side side, Price price) {
    if (side == Side::BUY) {
        auto it = bids_.find(price);
        if (it != bids_.end()) {
            return it->second;
        }
        PriceLevel* newLevel = levelPool_.allocate(price);
        bids_[price] = newLevel;
        return newLevel;
    } else {
        auto it = asks_.find(price);
        if (it != asks_.end()) {
            return it->second;
        }
        PriceLevel* newLevel = levelPool_.allocate(price);
        asks_[price] = newLevel;
        return newLevel;
    }
}

void OrderBook::removePriceLevelIfEmpty(Side side, Price price, PriceLevel* level) {
    if (!level || !level->empty()) return;

    if (side == Side::BUY) {
        bids_.erase(price);
    } else {
        asks_.erase(price);
    }
    levelPool_.deallocate(level);
}

void OrderBook::printBook(size_t maxLevels) const {
    std::cout << "\n------------------ MARKET DEPTH ------------------\n";
    std::cout << " ASKS (SELLS)\n";
    size_t count = 0;

    // Print Asks top-down (highest ask down to lowest ask)
    std::vector<std::pair<Price, Quantity>> askList;
    for (auto it = asks_.begin(); it != asks_.end() && count < maxLevels; ++it, ++count) {
        askList.push_back({it->first, it->second->totalVolume});
    }
    for (auto it = askList.rbegin(); it != askList.rend(); ++it) {
        std::cout << "   Price: " << std::setw(8) << it->first
                  << " | Vol: " << std::setw(6) << it->second << "\n";
    }

    Price spread = 0;
    if (getSpread(spread)) {
        std::cout << "   ---------------- SPREAD: " << spread << " --------------- \n";
    } else {
        std::cout << "   ---------------- SPREAD: N/A -------------- \n";
    }

    std::cout << " BIDS (BUYS)\n";
    count = 0;
    for (auto it = bids_.begin(); it != bids_.end() && count < maxLevels; ++it, ++count) {
        std::cout << "   Price: " << std::setw(8) << it->first
                  << " | Vol: " << std::setw(6) << it->second->totalVolume << "\n";
    }
    std::cout << "--------------------------------------------------\n";
}

} // namespace lob
