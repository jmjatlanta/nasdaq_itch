#pragma once
#include <cstdint>
#include <cstring>
#include <climits>
#include <string>
#include <memory>

namespace itch
{

struct message_record {
    enum class field_type {
        ALPHA = 0,
        INTEGER = 1,
        PRICE4 = 2,
        PRICE8 = 3,
    };
    uint8_t offset = 0;
    uint8_t length = 0;
    field_type type;
};

template <const int NUM_BYTES>
std::shared_ptr<char[]> swap_endian_bytes(const void* in)
{
    std::shared_ptr<char[]> buffer(new char[8]);
    const char* incoming = (const char*)in;
    static_assert(CHAR_BIT == 8, "CHAR_BIT != 8");
    for(size_t k = 0; k < NUM_BYTES; k++)
        buffer[k] = incoming[NUM_BYTES - k - 1];
    return buffer;
}

template<unsigned int SIZE>
struct message {
    char message_type = ' ';
    message(char message_type) : message_type(message_type) 
    {
        record[0] = message_type;
        memset( &record[1], 0, SIZE-1 );
    }
    message(const char* in)
    {
        memcpy(record, in, SIZE);
    }
    const uint8_t get_raw_byte(uint8_t pos) const { return record[pos]; }
    void set_raw_byte(uint8_t pos, uint8_t in) { record[pos] = in; }
    int64_t get_int(const message_record& mr) {
        // how many bytes to grab
        switch(mr.length)
        {
            case 2:
                return (int64_t)*swap_endian_bytes<2>(&record[mr.offset]).get();
            case 4:
                return (int64_t)*swap_endian_bytes<4>(&record[mr.offset]).get();
            case 6:
                return (int64_t)*swap_endian_bytes<6>(&record[mr.offset]).get();
            case 8:
                return (int64_t)*swap_endian_bytes<8>(&record[mr.offset]).get();
            default:
                break;
        }
        return 0;
    }
    void set_int(const message_record& mr, int64_t in)
    {
        switch(mr.length)
        {
            case 1:
                memcpy(&record[mr.offset], &in, mr.length);
                break;
            case 2:
                memcpy(&record[mr.offset], swap_endian_bytes<2>(&in).get(), mr.length);
                break;
            case 4:
                memcpy(&record[mr.offset], swap_endian_bytes<4>(&in).get(), mr.length);
                break;
            case 6:
                memcpy(&record[mr.offset], swap_endian_bytes<6>(&in).get(), mr.length);
                break;
            case 8:
                memcpy(&record[mr.offset], swap_endian_bytes<8>(&in).get(), mr.length);
                break;
            default:
                break;
        }
    }
    void set_string(const message_record& mr, const std::string& in)
    {
        strncpy(&record[mr.offset], in.c_str(), mr.length);
    }
    const std::string get_string(const message_record& mr)
    {
        // get the section of the record we want
        char buf[mr.length+1];
        memset(buf, 0, mr.length+1);
        strncpy(buf, &record[mr.offset], mr.length);
        return buf;
    }
    const char* get_record() const { return record; }
    protected:
    char record[SIZE];
};

const static int8_t SYSTEM_EVENT_LEN = 12;
struct system_event : public message<SYSTEM_EVENT_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA};
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::INTEGER};
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record EVENT_CODE{11, 1, message_record::field_type::ALPHA};

    system_event() : message('S') { }
    system_event(const char* in) : message(in) {}
};

const static int8_t STOCK_DIRECTORY_LEN = 39;
struct stock_directory : public message<STOCK_DIRECTORY_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA};
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::INTEGER};
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record STOCK{11, 8, message_record::field_type::ALPHA};
    static constexpr message_record FINANCIAL_STATUS_INDICATOR{20, 1, message_record::field_type::ALPHA};
    static constexpr message_record ROUND_LOT_SIZE{21, 4, message_record::field_type::INTEGER};
    static constexpr message_record ROUND_LOTS_ONLY{25, 1, message_record::field_type::ALPHA};
    static constexpr message_record ISSUE_CLASSIFICATION{26, 1, message_record::field_type::ALPHA};
    static constexpr message_record ISSUE_SUB_TYPE{27, 2, message_record::field_type::ALPHA};
    static constexpr message_record AUTHENTICITY{29, 1, message_record::field_type::ALPHA};
    static constexpr message_record SHORT_SALE_THRESHOLD_INDICATOR{30, 1, message_record::field_type::ALPHA};
    static constexpr message_record IPO_FLAG{31, 1, message_record::field_type::ALPHA};
    static constexpr message_record LULDREFERENCE_PRICE_TIER{32, 1, message_record::field_type::ALPHA};
    static constexpr message_record ETP_FLAG{33, 1, message_record::field_type::ALPHA};
    static constexpr message_record ETP_LEVERAGE_FACTOR{34, 4, message_record::field_type::INTEGER};
    static constexpr message_record INVERSE_INDICATOR{38, 1, message_record::field_type::ALPHA};
    
    stock_directory() : message('R') {}
    stock_directory(const char* in) : message(in) {}
};

const static int8_t STOCK_TRADING_ACTION_LEN = 25;
struct stock_trading_action : public message<STOCK_TRADING_ACTION_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA};
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::INTEGER};
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record STOCK{11, 8, message_record::field_type::ALPHA};
    static constexpr message_record TRADING_STATE{19, 1, message_record::field_type::ALPHA};
    static constexpr message_record RESERVED{20, 1, message_record::field_type::ALPHA};
    static constexpr message_record REASON{21, 4, message_record::field_type::ALPHA};

    stock_trading_action() : message('H') {}
    stock_trading_action(const char* in) : message(in) {}
};

const static int8_t REG_SHO_RESTRICTION_LEN = 20;
struct reg_sho_restriction : public message<REG_SHO_RESTRICTION_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA};
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::INTEGER};
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record STOCK{11, 8, message_record::field_type::ALPHA};
    static constexpr message_record REG_SHO_ACTION{19, 1, message_record::field_type::ALPHA};

    reg_sho_restriction() : message('Y') {}
    reg_sho_restriction(const char* in) : message(in) {}
};

const static int8_t MARKET_PARTICIPANT_POSITION_LEN = 26;
struct market_participant_position : public message<MARKET_PARTICIPANT_POSITION_LEN> { 
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA}; 
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::INTEGER};
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record MPID{11, 4, message_record::field_type::ALPHA};
    static constexpr message_record STOCK{15, 8, message_record::field_type::ALPHA};
    static constexpr message_record PRIMARY_MARKET_MAKER{23, 1, message_record::field_type::ALPHA};
    static constexpr message_record MARKET_MAKER_MODE{24, 1, message_record::field_type::ALPHA};
    static constexpr message_record MARKET_PARTICIPANT_STATE{25, 1, message_record::field_type::ALPHA};

    market_participant_position() : message('L') {}
    market_participant_position(const char* in) : message(in) {}
};

const static int8_t MWCP_DECLINE_LEVEL_LEN = 35;
struct mwcp_decline_level : public message<MWCP_DECLINE_LEVEL_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA}; 
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::ALPHA}; 
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record LEVEL_1{11, 8, message_record::field_type::PRICE8};
    static constexpr message_record LEVEL_2{19, 8, message_record::field_type::PRICE8};
    static constexpr message_record LEVEL_3{27, 8, message_record::field_type::PRICE8};

    mwcp_decline_level() : message('V') {}
    mwcp_decline_level(const char* in) : message(in) {}
};

const static int8_t MWCP_STATUS_LEN = 12;
struct mwcp_status : public message<MWCP_STATUS_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA}; 
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::ALPHA}; 
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record BREACHED_LEVEL{11, 1, message_record::field_type::ALPHA};

    mwcp_status() : message('W') {}
    mwcp_status(const char* in) : message(in) {}
};

const static int8_t IPO_QUOTING_PERIOD_UPDATE_LEN = 28;
struct ipo_quoting_period_update : public message<IPO_QUOTING_PERIOD_UPDATE_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA}; 
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::ALPHA}; 
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record STOCK{11, 8, message_record::field_type::ALPHA};
    static constexpr message_record IPO_QUOTATION_RELEASE_TIME{19, 4, message_record::field_type::INTEGER};
    static constexpr message_record IPO_QUOTATION_RELEASE_QUALIFIER{23, 1, message_record::field_type::ALPHA};
    static constexpr message_record IPO_PRICE{24, 4, message_record::field_type::PRICE4};

    ipo_quoting_period_update() : message('K') {}
    ipo_quoting_period_update(const char* in) : message(in) {}
};
    
const static int8_t LULD_AUCTION_COLLAR_LEN = 35;
struct luld_auction_collar : public message<LULD_AUCTION_COLLAR_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA}; 
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::ALPHA}; 
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record STOCK{11, 8, message_record::field_type::ALPHA};
    static constexpr message_record AUCTION_COLLAR_REFERENCE_PRICE{19, 4, message_record::field_type::PRICE4};
    static constexpr message_record UPPER_AUCTION_COLLAR_PRICE{23, 4, message_record::field_type::PRICE4};
    static constexpr message_record LOWER_AUCTION_COLLAR_PRICE{27, 4, message_record::field_type::PRICE4};
    static constexpr message_record AUCTION_COLLAR_EXTENSION{31, 4, message_record::field_type::INTEGER};

    luld_auction_collar() : message('J') {}
    luld_auction_collar(const char* in) : message(in) {}
};

const static int8_t OPERATIONAL_HALT_LEN = 21;
struct operational_halt : public message<OPERATIONAL_HALT_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA}; 
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::ALPHA}; 
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record STOCK{11, 8, message_record::field_type::ALPHA};
    static constexpr message_record MARKET_CODE{19, 1, message_record::field_type::ALPHA};
    static constexpr message_record OPERATIONAL_HALT_ACTION{20, 1, message_record::field_type::ALPHA};

    operational_halt() : message('h') {}
    operational_halt(const char* in) : message(in) {}
};

const static int8_t ADD_ORDER_LEN = 36;
struct add_order : public message<ADD_ORDER_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA}; 
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::ALPHA}; 
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record ORDER_REFERENCE_NUMBER{11, 8, message_record::field_type::INTEGER};
    static constexpr message_record BUY_SELL_INDICATOR{19, 1, message_record::field_type::ALPHA};
    static constexpr message_record SHARES{20, 4, message_record::field_type::INTEGER};
    static constexpr message_record STOCK{24, 8, message_record::field_type::ALPHA};
    static constexpr message_record PRICE{32, 4, message_record::field_type::PRICE4};

    add_order() : message('A') {}
    add_order(const char* in) : message(in) {}
};

const static int8_t ADD_ORDER_WITH_MPID_LEN = 40;
struct add_order_with_mpid : public message<ADD_ORDER_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA}; 
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::ALPHA}; 
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record ORDER_REFERENCE_NUMBER{11, 8, message_record::field_type::INTEGER};
    static constexpr message_record BUY_SELL_INDICATOR{19, 1, message_record::field_type::ALPHA};
    static constexpr message_record SHARES{20, 4, message_record::field_type::INTEGER};
    static constexpr message_record STOCK{24, 8, message_record::field_type::ALPHA};
    static constexpr message_record PRICE{32, 4, message_record::field_type::PRICE4};
    static constexpr message_record ATTRIBUTION{36, 4, message_record::field_type::ALPHA};

    add_order_with_mpid() : message('F') {}
    add_order_with_mpid(const char* in) : message(in) {}
};

const static int8_t ORDER_EXECUTED_LEN = 31;
struct order_executed : public message<ORDER_EXECUTED_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA}; 
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::ALPHA}; 
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record ORDER_REFERENCE_NUMBER{11, 8, message_record::field_type::INTEGER};
    static constexpr message_record EXECUTED_SHARES{19, 4, message_record::field_type::INTEGER};
    static constexpr message_record MATCH_NUMBER{23, 8, message_record::field_type::INTEGER};

    order_executed() : message('E') {}
    order_executed(const char* in) : message(in) {}
};

const static int8_t ORDER_EXECUTED_WITH_PRICE_LEN = 36;
struct order_executed_with_price : public message<ORDER_EXECUTED_WITH_PRICE_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA}; 
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::ALPHA}; 
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record ORDER_REFERENCE_NUMBER{11, 8, message_record::field_type::INTEGER};
    static constexpr message_record EXECUTED_SHARES{19, 4, message_record::field_type::INTEGER};
    static constexpr message_record MATCH_NUMBER{23, 8, message_record::field_type::INTEGER};
    static constexpr message_record PRINTABLE{31, 1, message_record::field_type::ALPHA};
    static constexpr message_record EXECUTION_PRICE{32, 4, message_record::field_type::PRICE4};

    order_executed_with_price() : message('C') {}
    order_executed_with_price(const char* in) : message(in) {}
};

const static int8_t ORDER_CANCEL_LEN = 23;
struct order_cancel : public message<ORDER_CANCEL_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA}; 
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::ALPHA}; 
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record ORDER_REFERENCE_NUMBER{11, 8, message_record::field_type::INTEGER};
    static constexpr message_record CANCELLED_SHARES{19, 4, message_record::field_type::INTEGER};

    order_cancel() : message('X') {}
    order_cancel(const char* in) : message(in) {}
};

const static int8_t ORDER_DELETE_LEN = 19;
struct order_delete : public message<ORDER_DELETE_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA}; 
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::ALPHA}; 
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record ORDER_REFERENCE_NUMBER{11, 8, message_record::field_type::INTEGER};

    order_delete() : message('D') {}
    order_delete(const char* in) : message(in) {}
};

const static int8_t ORDER_REPLACE_LEN = 35;
struct order_replace : public message<ORDER_REPLACE_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA}; 
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::ALPHA}; 
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record ORIGINAL_ORDER_REFERENCE_NUMBER{11, 8, message_record::field_type::INTEGER};
    static constexpr message_record NEW_ORDER_REFERENCE_NUMBER{19, 8, message_record::field_type::INTEGER};
    static constexpr message_record SHARES{27, 4, message_record::field_type::INTEGER};
    static constexpr message_record PRICE{31, 4, message_record::field_type::PRICE4};

    order_replace() : message('U') {}
    order_replace(const char* in) : message(in) {}
};

const static int8_t TRADE_LEN = 35;
struct trade : public message<TRADE_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA}; 
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::ALPHA}; 
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record ORDER_REFERENCE_NUMBER{11, 8, message_record::field_type::INTEGER};
    static constexpr message_record BUY_SELL_INDICATOR{19, 1, message_record::field_type::ALPHA};
    static constexpr message_record SHARES{20, 4, message_record::field_type::INTEGER};
    static constexpr message_record STOCK{24, 8, message_record::field_type::ALPHA};
    static constexpr message_record PRICE{32, 4, message_record::field_type::PRICE4};
    static constexpr message_record MATCH_NUMBER{36, 8, message_record::field_type::INTEGER};

    trade() : message('P') {}
    trade(const char* in) : message(in) {}
};

const static int8_t TRADE_NON_CROSS_LEN = 40;
struct trade_non_cross : public message<TRADE_NON_CROSS_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA}; 
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::ALPHA}; 
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record SHARES{11, 4, message_record::field_type::INTEGER};
    static constexpr message_record STOCK{19, 8, message_record::field_type::ALPHA};
    static constexpr message_record CROSS_PRICE{27, 4, message_record::field_type::PRICE4};
    static constexpr message_record MATCH_NUMBER{31, 8, message_record::field_type::INTEGER};
    static constexpr message_record CROSS_TYPE{39, 1, message_record::field_type::ALPHA};

    trade_non_cross() : message('Q') {}
    trade_non_cross(const char* in) : message(in) {}
};

const static int8_t BROKEN_TRADE_LEN = 19;
struct broken_trade : public message<BROKEN_TRADE_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA}; 
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::ALPHA}; 
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record MATCH_NUMBER{11, 8, message_record::field_type::INTEGER};

    broken_trade() : message('B') {}
    broken_trade(const char* in) : message(in) {}
};

const static int8_t NOII_LEN = 19;
struct noii : public message<NOII_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA}; 
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::ALPHA}; 
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record PAIRED_SHARES{11, 8, message_record::field_type::INTEGER};
    static constexpr message_record IMBALANCE_SHARES{19, 8, message_record::field_type::INTEGER};
    static constexpr message_record IMBALANCE_DIRECTION{27, 1, message_record::field_type::ALPHA};
    static constexpr message_record STOCK{28, 8, message_record::field_type::ALPHA};
    static constexpr message_record FAR_PRICE{36, 4, message_record::field_type::PRICE4};
    static constexpr message_record NEAR_PRICE{40, 4, message_record::field_type::PRICE4};
    static constexpr message_record CURRENT_REFERENCE_PRICE{44, 4, message_record::field_type::PRICE4};
    static constexpr message_record CROSS_TYPE{48, 1, message_record::field_type::ALPHA};
    static constexpr message_record PRICE_VARIATION_INDICATOR{49, 1, message_record::field_type::ALPHA};

    noii() : message('I') {}
    noii(const char* in) : message(in) {}
};

const static int8_t RPII_LEN = 20;
struct rpii : public message<RPII_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA}; 
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::ALPHA}; 
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record STOCK{11, 8, message_record::field_type::ALPHA};
    static constexpr message_record INTEREST_FLAG{19, 1, message_record::field_type::ALPHA};

    rpii() : message('N') {}
    rpii(const char* in) : message(in) {}
};

const static int8_t DIRECT_LISTING_WITH_CAPITAL_RAISE_PRICE_DISCOVERY_LEN = 48;
struct direct_listing_with_capital_raise_price_discovery : 
        public message<DIRECT_LISTING_WITH_CAPITAL_RAISE_PRICE_DISCOVERY_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA}; 
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::ALPHA}; 
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record STOCK{11, 8, message_record::field_type::ALPHA};
    static constexpr message_record OPEN_ELIGIBILITY_STATUS{19, 1, message_record::field_type::ALPHA};
    static constexpr message_record MINIMUM_ALLOWABLE_PRICE{20, 4, message_record::field_type::PRICE4};
    static constexpr message_record MAXIMUM_ALLOWABLE_PRICE{24, 4, message_record::field_type::PRICE4};
    static constexpr message_record NEAR_EXECUTION_PRICE{28, 4, message_record::field_type::PRICE4};
    static constexpr message_record NEAR_EXECUTION_TIME{32, 8, message_record::field_type::INTEGER};
    static constexpr message_record LOWER_PRICE_RANGE_COLLAR{40, 4, message_record::field_type::PRICE4};
    static constexpr message_record UPPER_PRICE_RANGE_COLLAR{44, 4, message_record::field_type::PRICE4};

    direct_listing_with_capital_raise_price_discovery() : message('O') {}
    direct_listing_with_capital_raise_price_discovery(const char* in) : message(in) {}
};

} // end namespace itch
