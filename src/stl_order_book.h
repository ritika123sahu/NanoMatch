#pragma once

#include "order.h"
#include <map>
#include <list>
#include <unordered_map>

namespace NanoMatch {

/**
 * @brief A "Slow" baseline implementation using standard STL containers.
 * This represents how a typical competitive programmer might build a LOB,
 * but with high cache misses and allocation overhead.
 */
class STLOrderBook {
public:
    void limit_order(uint64_t id, Side side, uint64_t price, uint32_t quantity) {
        if (side == Side::Buy) {
            auto it = asks.begin();
            while (quantity > 0 && it != asks.end() && price >= it->first) {
                auto& list = it->second;
                auto order_it = list.begin();
                while (order_it != list.end() && quantity > 0) {
                    uint32_t match_qty = std::min(quantity, order_it->quantity);
                    order_it->quantity -= match_qty;
                    quantity -= match_qty;
                    if (order_it->quantity == 0) order_it = list.erase(order_it);
                    else break;
                }
                if (list.empty()) it = asks.erase(it);
                else break;
            }
        } else {
            auto it = bids.rbegin();
            while (quantity > 0 && it != bids.rend() && price <= it->first) {
                auto& list = it->second;
                auto order_it = list.begin();
                while (order_it != list.end() && quantity > 0) {
                    uint32_t match_qty = std::min(quantity, order_it->quantity);
                    order_it->quantity -= match_qty;
                    quantity -= match_qty;
                    if (order_it->quantity == 0) order_it = list.erase(order_it);
                    else break;
                }
                if (list.empty()) {
                    auto erase_it = std::next(it).base();
                    bids.erase(erase_it);
                    it = bids.rbegin();
                } else break;
            }
        }

        if (quantity > 0) {
            Order* o = new Order{id, price, quantity, 0, side};
            if (side == Side::Buy) bids[price].push_back(*o);
            else asks[price].push_back(*o);
            order_map[id] = o;
        }
    }

    void cancel_order(uint64_t id) {
        if (order_map.count(id)) {
            Order* o = order_map[id];
            auto& list = (o->side == Side::Buy) ? bids[o->price] : asks[o->price];
            for (auto it = list.begin(); it != list.end(); ++it) {
                if (it->order_id == id) {
                    list.erase(it);
                    break;
                }
            }
            delete o;
            order_map.erase(id);
        }
    }

private:
    std::map<uint64_t, std::list<Order>> bids; // Sorted
    std::map<uint64_t, std::list<Order>> asks; // Sorted
    std::unordered_map<uint64_t, Order*> order_map;
};

} // namespace NanoMatch
