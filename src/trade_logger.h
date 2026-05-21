#pragma once

#include "order.h"
#include "fixed_ring_buffer.h"
#include <thread>
#include <atomic>
#include <fstream>

namespace NanoMatch {

/**
 * @brief Asynchronous trade logger using a lock-free SPSC ring buffer.
 * 
 * The matching engine (producer) pushes trade records into the buffer.
 * A background thread (consumer) drains the buffer and writes to a file.
 */
class TradeLogger {
public:
    TradeLogger(const std::string& filename) 
        : out_(filename), running_(true) {
        worker_ = std::thread(&TradeLogger::process_logs, this);
    }

    ~TradeLogger() {
        running_ = false;
        if (worker_.joinable()) {
            worker_.join();
        }
    }

    // Producer side: Called by the matching engine. Lock-free.
    void log_trade(const Trade& trade) {
        while (!buffer_.push(trade)) {
            // Buffer full: in a real HFT system, we might drop logs 
            // or have a larger buffer to avoid blocking the engine.
            std::this_thread::yield(); 
        }
    }

private:
    void process_logs() {
        Trade trade;
        while (running_ || buffer_.size() > 0) {
            if (buffer_.pop(trade)) {
                out_ << trade.timestamp << ","
                     << trade.buyer_order_id << ","
                     << trade.seller_order_id << ","
                     << trade.price << ","
                     << trade.quantity << "\n";
            } else {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
        }
        out_.flush();
    }

    std::ofstream out_;
    RingBuffer<Trade, 65536> buffer_; // 64K entries
    std::thread worker_;
    std::atomic<bool> running_;
};

} // namespace NanoMatch
