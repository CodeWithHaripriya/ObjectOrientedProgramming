#include "OrderBook.hpp"
#include <iostream>
#include <cassert>

void testPriceTimePriority() {
    lob::OrderBook book;

    // Two Sell orders at same price level (100)
    book.addOrder(1, lob::Side::SELL, 100, 50); // First
    book.addOrder(2, lob::Side::SELL, 100, 50); // Second

    // Incoming Buy order for 60 units
    auto matches = book.addOrder(3, lob::Side::BUY, 100, 60);

    assert(matches.size() == 2);
    // First match must be with Order 1 (FIFO priority)
    assert(matches[0].makerOrderId == 1);
    assert(matches[0].matchQuantity == 50);

    // Second match must be with Order 2 for remaining 10 units
    assert(matches[1].makerOrderId == 2);
    assert(matches[1].matchQuantity == 10);

    std::cout << "[PASS] testPriceTimePriority\n";
}

void testPartialAndFullFills() {
    lob::OrderBook book;

    book.addOrder(10, lob::Side::BUY, 150, 100);
    auto matches = book.addOrder(20, lob::Side::SELL, 150, 40);

    assert(matches.size() == 1);
    assert(matches[0].matchQuantity == 40);
    assert(book.getActiveOrdersCount() == 1);

    lob::Price bestBidPrice = 0;
    lob::Quantity bestBidQty = 0;
    assert(book.getBestBid(bestBidPrice, bestBidQty));
    assert(bestBidPrice == 150);
    assert(bestBidQty == 60); // 100 - 40 remaining

    std::cout << "[PASS] testPartialAndFullFills\n";
}

void testOrderCancellation() {
    lob::OrderBook book;

    book.addOrder(100, lob::Side::BUY, 200, 500);
    assert(book.getActiveOrdersCount() == 1);

    bool canceled = book.cancelOrder(100);
    assert(canceled);
    assert(book.getActiveOrdersCount() == 0);

    lob::Price bestBidPrice = 0;
    lob::Quantity bestBidQty = 0;
    assert(!book.getBestBid(bestBidPrice, bestBidQty)); // Book should be empty

    std::cout << "[PASS] testOrderCancellation\n";
}

int main() {
    std::cout << "Running LOB-Engine Unit Test Suite...\n";
    testPriceTimePriority();
    testPartialAndFullFills();
    testOrderCancellation();
    std::cout << "ALL LOB-ENGINE UNIT TESTS PASSED SUCCESSFULLY!\n";
    return 0;
}
