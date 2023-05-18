#pragma once
#include <cstdint>
#include <cstring>
#include <climits>
#include <string>
#include <memory>
#include <vector>

namespace ouch
{

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

struct message_record {
    enum class field_type {
        ALPHA = 0,
        INTEGER = 1,
        PRICE4 = 2,
        PRICE8 = 3,
        USER_REF_NUM = 4,
        VAR = 5,
    };
    uint8_t offset = 0;
    uint8_t length = 0;
    field_type type;
};

template<unsigned int SIZE>
struct message {
    const char message_type = ' ';
    const message_record* variable_field_record = nullptr;
    message(char message_type, const message_record* variable_field) 
            : message_type(message_type), variable_field_record(variable_field)
    {
        record[0] = message_type;
        memset( &record[1], 0, SIZE-1 );
    }
    message(const char* in, const message_record* variable_field) 
            : message_type(in[0]), variable_field_record(variable_field)
    {
        memcpy(record, in, SIZE);
    }
    ~message() {
        if (appendages != nullptr)
            delete appendages;
    }
    void set_appendages(char* in) {
        if (variable_field_record != nullptr)
        {
            int64_t sz = get_int(*variable_field_record);
            appendages = (char*)malloc(sz);
            memcpy(appendages, in, sz);
        }
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
    const char* get_appendages() const { return appendages; }
    protected:
    char record[SIZE];
    char* appendages = nullptr;
};

const static int8_t ENTER_ORDER_FIXED_LEN = 47; // NOTE: has 1 variable length field at pos 47
struct enter_order : public message<ENTER_ORDER_FIXED_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA};
    static constexpr message_record USER_REF_NUM{1, 4, message_record::field_type::USER_REF_NUM};
    static constexpr message_record SIDE{5, 1, message_record::field_type::ALPHA};
    static constexpr message_record QUANTITY{6, 4, message_record::field_type::INTEGER};
    static constexpr message_record SYMBOL{10, 8, message_record::field_type::ALPHA};
    static constexpr message_record PRICE{18, 8, message_record::field_type::PRICE8};
    static constexpr message_record TIME_IN_FORCE{26, 1, message_record::field_type::ALPHA};
    static constexpr message_record DISPLAY{27, 1, message_record::field_type::ALPHA};
    static constexpr message_record CAPACITY{28, 1, message_record::field_type::ALPHA};
    static constexpr message_record INTERMARKET_SWEEP_ELIGIBILITY{29, 1, message_record::field_type::ALPHA};
    static constexpr message_record CROSS_TYPE{30, 1, message_record::field_type::ALPHA};
    static constexpr message_record CI_ORD_ID{31, 14, message_record::field_type::ALPHA};
    static constexpr message_record APPENDAGE_LENGTH{45, 2, message_record::field_type::INTEGER};

    enter_order() : message('O', &APPENDAGE_LENGTH) { }
    enter_order(const char* in) : message(in, &APPENDAGE_LENGTH) {}
};

const static int8_t REPLACE_ORDER_FIXED_LEN = 40; // NOTE: has 1 variable length field
struct replace_order : public message<REPLACE_ORDER_FIXED_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA};
    static constexpr message_record ORIG_USER_REF_NUM{1, 4, message_record::field_type::USER_REF_NUM};
    static constexpr message_record USER_REF_NUM{5, 4, message_record::field_type::USER_REF_NUM};
    static constexpr message_record QUANTITY{9, 4, message_record::field_type::INTEGER};
    static constexpr message_record PRICE{13, 8, message_record::field_type::PRICE8};
    static constexpr message_record TIME_IN_FORCE{21, 1, message_record::field_type::ALPHA};
    static constexpr message_record DISPLAY{22, 1, message_record::field_type::ALPHA};
    static constexpr message_record INTERMARKET_SWEEP_ELIGIBILITY{23, 1, message_record::field_type::ALPHA};
    static constexpr message_record CI_ORD_ID{24, 14, message_record::field_type::ALPHA};
    static constexpr message_record APPENDAGE_LENGTH{38, 2, message_record::field_type::INTEGER};

    replace_order() : message('U', &APPENDAGE_LENGTH) { }
    replace_order(const char* in) : message(in, &APPENDAGE_LENGTH) {}
};

const static int8_t CANCEL_ORDER_FIXED_LEN = 9;
struct cancel_order : public message<CANCEL_ORDER_FIXED_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA};
    static constexpr message_record USER_REF_NUM{1, 4, message_record::field_type::USER_REF_NUM};
    static constexpr message_record QUANTITY{5, 4, message_record::field_type::INTEGER};

    cancel_order() : message('X', nullptr) { }
    cancel_order(const char* in) : message(in, nullptr) {}
};

const static int8_t MODIFY_ORDER_FIXED_LEN = 10;
struct modify_order : public message<MODIFY_ORDER_FIXED_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA};
    static constexpr message_record USER_REF_NUM{1, 4, message_record::field_type::USER_REF_NUM};
    static constexpr message_record SIDE{5, 1, message_record::field_type::ALPHA};
    static constexpr message_record QUANTITY{6, 4, message_record::field_type::INTEGER};

    modify_order() : message('M', nullptr) { }
    modify_order(const char* in) : message(in, nullptr) {}
};

const static int8_t ACCOUNT_QUERY_FIXED_LEN = 10;
struct account_query : public message<ACCOUNT_QUERY_FIXED_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA};

    account_query() : message('Q', nullptr) { }
    account_query(const char* in) : message(in, nullptr) {}
};

/****
 * The following messages are outbound messages
 */

const static int8_t SYSTEM_EVENT_FIXED_LEN = 10;
struct system_event : public message<SYSTEM_EVENT_FIXED_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA};
    static constexpr message_record TIMESTAMP{1, 8, message_record::field_type::INTEGER};
    static constexpr message_record EVENT_CODE{9, 1, message_record::field_type::ALPHA};

    system_event() : message('S', nullptr) { }
    system_event(const char* in) : message(in, nullptr) {}
};

const static int8_t ORDER_ACCEPTED_FIXED_LEN = 64; // NOTE: has 1 variable length field
struct order_accepted : public message<ORDER_ACCEPTED_FIXED_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA};
    static constexpr message_record TIMESTAMP{1, 8, message_record::field_type::INTEGER};
    static constexpr message_record USER_REF_NUM{9, 4, message_record::field_type::USER_REF_NUM};
    static constexpr message_record SIDE{13, 1, message_record::field_type::ALPHA};
    static constexpr message_record QUANTITY{14, 4, message_record::field_type::INTEGER};
    static constexpr message_record SYMBOL{18, 8, message_record::field_type::ALPHA};
    static constexpr message_record PRICE{26, 8, message_record::field_type::PRICE8};
    static constexpr message_record TIME_IN_FORCE{34, 1, message_record::field_type::ALPHA};
    static constexpr message_record DISPLAY{35, 1, message_record::field_type::ALPHA};
    static constexpr message_record ORDER_REFERENCE_NUMBER{36, 8, message_record::field_type::INTEGER};
    static constexpr message_record CAPACITY{44, 1, message_record::field_type::ALPHA};
    static constexpr message_record INTERMARKET_SWEEP_ELIGIBILITY{45, 1, message_record::field_type::ALPHA};
    static constexpr message_record CROSS_TYPE{46, 1, message_record::field_type::ALPHA};
    static constexpr message_record ORDER_STATE{47, 1, message_record::field_type::ALPHA};
    static constexpr message_record CI_ORD_ID{24, 14, message_record::field_type::ALPHA};
    static constexpr message_record APPENDAGE_LENGTH{38, 2, message_record::field_type::INTEGER};

    order_accepted() : message('A', &APPENDAGE_LENGTH) { }
    order_accepted(const char* in) : message(in, &APPENDAGE_LENGTH) {}
};

const static int8_t ORDER_REPLACED_FIXED_LEN = 68; // NOTE: has 1 variable length field
struct order_replaced : public message<ORDER_REPLACED_FIXED_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA};
    static constexpr message_record TIMESTAMP{1, 8, message_record::field_type::INTEGER};
    static constexpr message_record ORIG_USER_REF_NUM{9, 4, message_record::field_type::USER_REF_NUM};
    static constexpr message_record USER_REF_NUM{13, 4, message_record::field_type::USER_REF_NUM};
    static constexpr message_record SIDE{17, 1, message_record::field_type::ALPHA};
    static constexpr message_record QUANTITY{18, 4, message_record::field_type::INTEGER};
    static constexpr message_record SYMBOL{22, 8, message_record::field_type::ALPHA};
    static constexpr message_record PRICE{30, 8, message_record::field_type::PRICE8};
    static constexpr message_record TIME_IN_FORCE{38, 1, message_record::field_type::ALPHA};
    static constexpr message_record DISPLAY{39, 1, message_record::field_type::ALPHA};
    static constexpr message_record ORDER_REFERENCE_NUMBER{40, 8, message_record::field_type::INTEGER};
    static constexpr message_record CAPACITY{48, 1, message_record::field_type::ALPHA};
    static constexpr message_record INTERMARKET_SWEEP_ELIGIBILITY{49, 1, message_record::field_type::ALPHA};
    static constexpr message_record CROSS_TYPE{50, 1, message_record::field_type::ALPHA};
    static constexpr message_record ORDER_STATE{51, 1, message_record::field_type::ALPHA};
    static constexpr message_record CI_ORD_ID{52, 14, message_record::field_type::ALPHA};
    static constexpr message_record APPENDAGE_LENGTH{66, 2, message_record::field_type::INTEGER};

    order_replaced() : message('U', &APPENDAGE_LENGTH) { }
    order_replaced(const char* in) : message(in, &APPENDAGE_LENGTH) {}
};

const static int8_t ORDER_CANCELED_FIXED_LEN = 18;
struct order_canceled : public message<ORDER_CANCELED_FIXED_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA};
    static constexpr message_record TIMESTAMP{1, 8, message_record::field_type::INTEGER};
    static constexpr message_record USER_REF_NUM{9, 4, message_record::field_type::USER_REF_NUM};
    static constexpr message_record QUANTITY{13, 4, message_record::field_type::INTEGER};
    static constexpr message_record REASON{17, 1, message_record::field_type::INTEGER};

    order_canceled() : message('C', nullptr) { }
    order_canceled(const char* in) : message(in, nullptr) {}
};

const static int8_t AIQ_CANCELED_FIXED_LEN = 31;
struct aiq_canceled : public message<AIQ_CANCELED_FIXED_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA};
    static constexpr message_record TIMESTAMP{1, 8, message_record::field_type::INTEGER};
    static constexpr message_record USER_REF_NUM{9, 4, message_record::field_type::USER_REF_NUM};
    static constexpr message_record DECREMENT_SHARES{13, 4, message_record::field_type::INTEGER};
    static constexpr message_record REASON{17, 1, message_record::field_type::INTEGER};
    static constexpr message_record QUANTITY_PREVENTED_FROM_TRADING{18, 4, message_record::field_type::INTEGER};
    static constexpr message_record EXECUTION_PRICE{22, 8, message_record::field_type::PRICE8};
    static constexpr message_record LIQUIDITY_FLAG{30, 1, message_record::field_type::ALPHA};

    aiq_canceled() : message('D', nullptr) { }
    aiq_canceled(const char* in) : message(in, nullptr) {}
};

const static int8_t ORDER_EXECUTED_FIXED_LEN = 36; // NOTE: has 1 variable length field
struct order_executed : public message<ORDER_EXECUTED_FIXED_LEN> {
    static constexpr message_record MESSAGE_TYPE{0, 1, message_record::field_type::ALPHA};
    static constexpr message_record TIMESTAMP{1, 8, message_record::field_type::INTEGER};
    static constexpr message_record USER_REF_NUM{9, 4, message_record::field_type::USER_REF_NUM};
    static constexpr message_record QUANTITY{13, 4, message_record::field_type::INTEGER};
    static constexpr message_record PRICE{17, 8, message_record::field_type::PRICE8};
    static constexpr message_record LIQUIDITY_FLAG{25, 1, message_record::field_type::ALPHA};
    static constexpr message_record MATCH_NUMBER{26, 8, message_record::field_type::INTEGER};
    static constexpr message_record APPENDAGE_LENGTH{34, 2, message_record::field_type::INTEGER};

    order_executed() : message('E', &APPENDAGE_LENGTH) { }
    order_executed(const char* in) : message(in, &APPENDAGE_LENGTH) {}
};

} // end namespace ouch
