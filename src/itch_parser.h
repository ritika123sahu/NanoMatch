#pragma once

#include <cstdint>
#include <string>
#include <functional>
#include "order.h"

namespace NanoMatch {

/**
 * @brief NASDAQ ITCH 5.0 Message Types (Partial list relevant to LOB)
 */
namespace ITCH {
    enum class MessageType : char {
        AddOrder = 'A',
        AddOrderMPID = 'F',
        OrderExecuted = 'E',
        OrderExecutedWithPrice = 'C',
        OrderCancel = 'X',
        OrderDelete = 'D',
        OrderReplace = 'U',
        Trade = 'P',
        BrokenTrade = 'B',
        Noii = 'I',
        Rpii = 'N'
    };

    #pragma pack(push, 1)
    struct AddOrderMsg {
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp; // 48-bit actually, needs careful parsing
        uint64_t order_reference_number;
        char buy_sell_indicator;
        uint32_t shares;
        char stock[8];
        uint32_t price;
    };

    struct OrderDeleteMsg {
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        uint64_t order_reference_number;
    };

    struct TradeMsg {
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        uint64_t order_reference_number;
        char buy_sell_indicator;
        uint32_t shares;
        char stock[8];
        uint32_t price;
        uint64_t match_number;
    };
    #pragma pack(pop)
}

class ITCHParser {
public:
    using MessageHandler = std::function<void(char type, const void* data)>;

    ITCHParser(MessageHandler handler) : handler_(std::move(handler)) {}

    // Maps the file and parses it message by message.
    void parse(const std::string& filename);

private:
    MessageHandler handler_;

    // Helper to decode 48-bit ITCH timestamp to nanoseconds
    static uint64_t parse_timestamp(const uint8_t* buf) {
        uint64_t ts = 0;
        for (int i = 0; i < 6; ++i) {
            ts = (ts << 8) | buf[i];
        }
        return ts;
    }
};

} // namespace NanoMatch
