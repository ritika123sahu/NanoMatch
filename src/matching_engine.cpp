#include "order_book.h"
#include "csv_parser.h"
#include "itch_parser.h"
#include <iostream>
#include <string>
#include <iomanip>

using namespace NanoMatch;

void print_summary(const OrderBook& book) {
    std::cout << "\n==========================================" << std::endl;
    std::cout << "          MATCHING ENGINE RESULTS         " << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << " Total Trades Executed: " << book.get_total_trades() << std::endl;
    std::cout << " Total Volume Traded:   " << book.get_total_volume() << std::endl;
    std::cout << "==========================================\n" << std::endl;

    std::cout << "--- FINAL ORDER BOOK STATE ---" << std::endl;
    const auto& asks = book.get_asks();
    const auto& bids = book.get_bids();

    std::cout << " [ASKS]" << std::endl;
    int count = 0;
    for (auto it = asks.begin(); it != asks.end() && count < 5; ++it, ++count) {
        std::cout << "   " << std::setw(8) << (*it)->price << " | " << (*it)->total_quantity << std::endl;
    }
    if (asks.empty()) std::cout << "   (Empty)" << std::endl;

    std::cout << " ------" << std::endl;

    count = 0;
    for (auto it = bids.begin(); it != bids.end() && count < 5; ++it, ++count) {
        std::cout << "   " << std::setw(8) << (*it)->price << " | " << (*it)->total_quantity << std::endl;
    }
    if (bids.empty()) std::cout << "   (Empty)" << std::endl;
    std::cout << " [BIDS]" << std::endl;
    std::cout << "==========================================\n" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: ./nanomatch <type: csv|itch> <filename> [nolog]" << std::endl;
        return 1;
    }

    std::string type = argv[1];
    std::string filename = argv[2];
    bool use_logger = (argc <= 3 || std::string(argv[3]) != "nolog");

    OrderBook book(use_logger ? std::make_unique<TradeLogger>("trades.csv") : nullptr);

    uint64_t order_count = 0;
    if (type == "csv") {
        std::cout << "Processing CSV dataset..." << std::endl;
        CSVParser parser([&book, &order_count](uint64_t ts, Side side, uint64_t price, uint32_t qty, char order_type) {
            order_count++;
            if (order_type == 'L') book.limit_order(order_count, side, price, qty);
            else book.market_order(order_count, side, qty);
            if (order_count % 100000 == 0) std::cout << "\r   Processed " << order_count << " orders..." << std::flush;
        });
        parser.parse(filename);
    }

    std::cout << "\n\nProcessing Complete!" << std::endl;
    print_summary(book);

    return 0;
}
