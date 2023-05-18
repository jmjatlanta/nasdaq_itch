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
        CHAR = 3,
        PRICE8 = 4,
    };
    uint8_t offset = 0;
    uint8_t length = 0;
    field_type type;
};

template <typename T>
T swap_endian(T u)
{
    static_assert(CHAR_BIT==8, "CHAR_BIT != 8");
    union {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;
    source.u = u;
    for(size_t k = 0; k < sizeof(T); k++)
        dest.u8[k] = source.u8[sizeof(T) - k - 1];
    return dest.u;
}

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
        char buf[mr.length+1] = {0};
        strncpy(buf, &record[mr.offset], mr.length);
        return buf;
    }
    const char* get_record() const { return record; }
    protected:
    char record[SIZE];
};

const static int8_t SYSTEM_EVENT_LEN = 12;
struct system_event_message : public message<SYSTEM_EVENT_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::CHAR};
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::INTEGER};
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record EVENT_CODE{11, 1, message_record::field_type::ALPHA};

    system_event_message() : message('S') { }
    system_event_message(const char* in) : message(in) {}
};

const static int8_t STOCK_DIRECTORY_LEN = 39;
struct stock_directory : public message<STOCK_DIRECTORY_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::CHAR};
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
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::CHAR};
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
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::CHAR};
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
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::CHAR}; 
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
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::CHAR}; 
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::CHAR}; 
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
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::CHAR}; 
    static constexpr message_record STOCK_LOCATE{1, 2, message_record::field_type::CHAR}; 
    static constexpr message_record TRACKING_NUMBER{3, 2, message_record::field_type::INTEGER};
    static constexpr message_record TIMESTAMP{5, 6, message_record::field_type::INTEGER};
    static constexpr message_record BREACHED_LEVEL{11, 1, message_record::field_type::ALPHA};

    mwcp_status() : message('W') {}
    mwcp_status(const char* in) : message(in) {}
};

} // end namespace itch
