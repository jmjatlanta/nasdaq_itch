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
        CHAR = 3,
        PRICE4 = 4,
        PRICE8 = 5,
        USER_REF_NUM = 6,
        VAR = 7,
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

} // end namespace ouch
