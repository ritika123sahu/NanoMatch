#include "order_book.h"
#include <chrono>

namespace NanoMatch {

OrderBook::OrderBook(std::unique_ptr<TradeLogger> logger) 
    : logger_(std::move(logger)) {
    bids.reserve(1000);
    asks.reserve(1000);
    order_map.reserve(100000);
}

OrderBook::~OrderBook() {}

void OrderBook::limit_order(uint64_t id, Side side, uint64_t price, uint32_t quantity) {
    if (side == Side::Buy) {
        while (quantity > 0 && !asks.empty() && price >= asks[0]->price) {
            PriceLevel* level = asks[0];
            Order* maker = level->head;
            while (maker && quantity > 0) {
                uint32_t match_qty = std::min(quantity, maker->quantity);
                execute_trade(maker, match_qty);
                
                maker->quantity -= match_qty;
                quantity -= match_qty;
                level->total_quantity -= match_qty;

                if (maker->quantity == 0) {
                    Order* to_remove = maker;
                    maker = maker->next;
                    
                    // Inline remove from book logic for hot path
                    order_map.erase(to_remove->order_id);
                    if (to_remove->prev) to_remove->prev->next = to_remove->next;
                    if (to_remove->next) to_remove->next->prev = to_remove->prev;
                    if (level->head == to_remove) level->head = to_remove->next;
                    if (level->tail == to_remove) level->tail = to_remove->prev;
                    
                    order_pool.deallocate(to_remove);
                } else {
                    break;
                }
            }
            if (level->total_quantity == 0) {
                asks.erase(asks.begin());
                level_pool.deallocate(level);
            }
        }
    } else { // Side::Sell
        while (quantity > 0 && !bids.empty() && price <= bids[0]->price) {
            PriceLevel* level = bids[0];
            Order* maker = level->head;
            while (maker && quantity > 0) {
                uint32_t match_qty = std::min(quantity, maker->quantity);
                execute_trade(maker, match_qty);

                maker->quantity -= match_qty;
                quantity -= match_qty;
                level->total_quantity -= match_qty;

                if (maker->quantity == 0) {
                    Order* to_remove = maker;
                    maker = maker->next;

                    order_map.erase(to_remove->order_id);
                    if (to_remove->prev) to_remove->prev->next = to_remove->next;
                    if (to_remove->next) to_remove->next->prev = to_remove->prev;
                    if (level->head == to_remove) level->head = to_remove->next;
                    if (level->tail == to_remove) level->tail = to_remove->prev;

                    order_pool.deallocate(to_remove);
                } else {
                    break;
                }
            }
            if (level->total_quantity == 0) {
                bids.erase(bids.begin());
                level_pool.deallocate(level);
            }
        }
    }

    if (quantity > 0) {
        Order* order = order_pool.allocate();
        order->order_id = id;
        order->side = side;
        order->price = price;
        order->quantity = quantity;
        order->timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        order->next = nullptr;
        order->prev = nullptr;
        add_to_book(order);
    }
}

void OrderBook::market_order(uint64_t id, Side side, uint32_t quantity) {
    if (side == Side::Buy) limit_order(id, side, UINT64_MAX, quantity);
    else limit_order(id, side, 0, quantity);
}

void OrderBook::cancel_order(uint64_t id) {
    auto it = order_map.find(id);
    if (__builtin_expect(it != order_map.end(), 1)) {
        remove_from_book(it->second);
    }
}

void OrderBook::add_to_book(Order* order) {
    PriceLevel* level = find_or_create_level(order->side, order->price);
    if (!level->head) {
        level->head = order;
        level->tail = order;
    } else {
        level->tail->next = order;
        order->prev = level->tail;
        level->tail = order;
    }
    level->total_quantity += order->quantity;
    order_map[order->order_id] = order;
}

void OrderBook::remove_from_book(Order* order) {
    if (order->prev) order->prev->next = order->next;
    if (order->next) order->next->prev = order->prev;

    auto& levels = (order->side == Side::Buy) ? bids : asks;
    auto it = std::lower_bound(levels.begin(), levels.end(), order->price, 
        [side = order->side](PriceLevel* pl, uint64_t p) {
            return (side == Side::Buy) ? pl->price > p : pl->price < p;
        });

    if (it != levels.end() && (*it)->price == order->price) {
        PriceLevel* level = *it;
        if (level->head == order) level->head = order->next;
        if (level->tail == order) level->tail = order->prev;
        level->total_quantity -= order->quantity;
        
        if (level->total_quantity == 0) {
            levels.erase(it);
            level_pool.deallocate(level);
        }
    }

    order_map.erase(order->order_id);
    order_pool.deallocate(order);
}

PriceLevel* OrderBook::find_or_create_level(Side side, uint64_t price) {
    auto& levels = (side == Side::Buy) ? bids : asks;
    auto it = std::lower_bound(levels.begin(), levels.end(), price, 
        [side](PriceLevel* pl, uint64_t p) {
            return (side == Side::Buy) ? pl->price > p : pl->price < p;
        });

    if (it != levels.end() && (*it)->price == price) return *it;
    
    PriceLevel* new_level = level_pool.allocate();
    new (new_level) PriceLevel(price);
    levels.insert(it, new_level);
    return new_level;
}

inline void OrderBook::execute_trade(Order* maker, uint32_t quantity) {
    total_trades++;
    total_volume += quantity;
    if (__builtin_expect(logger_ != nullptr, 0)) {
        Trade trade;
        trade.buyer_order_id = (maker->side == Side::Sell) ? 0 : maker->order_id;
        trade.seller_order_id = (maker->side == Side::Buy) ? 0 : maker->order_id;
        trade.price = maker->price;
        trade.quantity = quantity;
        trade.timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        logger_->log_trade(trade);
    }
}

} // namespace NanoMatch
