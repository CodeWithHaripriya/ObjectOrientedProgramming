#include "OrderBook.hpp"
#include <iostream>

int main() {
    std::cout << "==================================================\n";
    std::cout << "  LOB-Engine: C++20 Financial Matching Engine\n";
    std::cout << "==================================================\n";

    lob::OrderBook book;

    std::cout << "\n1. Submitting initial Limit Orders into the Book...\n";
    
    // Add Sell Orders (Asks)
    book.addOrder(101, lob::Side::SELL, 105, 50);
    book.addOrder(102, lob::Side::SELL, 106, 100);
    book.addOrder(103, lob::Side::SELL, 107, 200);

    // Add Buy Orders (Bids)
    book.addOrder(201, lob::Side::BUY, 103, 70);
    book.addOrder(202, lob::Side::BUY, 102, 120);
    book.addOrder(203, lob::Side::BUY, 100, 300);

    book.printBook(5);

    std::cout << "\n2. Submitting aggressive Aggressor Buy Order (Price 106, Qty 120)...\n";
    auto matches = book.addOrder(301, lob::Side::BUY, 106, 120);

    std::cout << "   -> Matched " << matches.size() << " trades:\n";
    for (const auto& match : matches) {
        std::cout << "      [TRADE] Maker Order #" << match.makerOrderId
                  << " <-> Taker Order #" << match.takerOrderId
                  << " @ Price " << match.matchPrice
                  << " | Qty " << match.matchQuantity << "\n";
    }

    book.printBook(5);

    std::cout << "\n3. Canceling Buy Order #201...\n";
    if (book.cancelOrder(201)) {
        std::cout << "   -> Order #201 successfully canceled in O(1) time.\n";
    }

    book.printBook(5);

    std::cout << "\nStats Summary:\n";
    std::cout << "  Total Trades Executed: " << book.getTotalTradesCount() << "\n";
    std::cout << "  Total Volume Traded:   " << book.getTotalVolumeTraded() << "\n";
    std::cout << "  Active Orders Remaining: " << book.getActiveOrdersCount() << "\n";

    return 0;
}
