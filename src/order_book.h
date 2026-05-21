#pragma once

#include "order.h"
#include "trade_logger.h"
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>

namespace NanoMatch {

struct PriceLevel {
    uint64_t price;
    Order* head = nullptr;
    Order* tail = nullptr;
    uint32_t total_quantity = 0;

    PriceLevel(uint64_t p) : price(p) {}
};

class OrderBook {
public:
    OrderBook(std::unique_ptr<TradeLogger> logger = nullptr);
    ~OrderBook();

    void limit_order(uint64_t id, Side side, uint64_t price, uint32_t quantity);
    void market_order(uint64_t id, Side side, uint32_t quantity);
    void cancel_order(uint64_t id);

    const std::vector<PriceLevel>& get_bids() const { return bids; }
    const std::vector<PriceLevel>& get_asks() const { return asks; }

    uint64_t get_total_trades() const { return total_trades; }
    uint64_t get_total_volume() const { return total_volume; }

private:
    void add_to_book(Order* order);
    void remove_from_book(Order* order);
    PriceLevel* find_or_create_level(Side side, uint64_t price);
    void execute_trade(Order* maker, Order* taker, uint32_t quantity);

    std::vector<PriceLevel> bids;
    std::vector<PriceLevel> asks;
    std::unordered_map<uint64_t, Order*> order_map;
    MemoryPool<Order> order_pool;
    std::unique_ptr<TradeLogger> logger_;

    uint64_t total_trades = 0;
    uint64_t total_volume = 0;
};

} // namespace NanoMatch
