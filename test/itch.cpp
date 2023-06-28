#include "itch.h"
#include <gtest/gtest.h>
#include <sstream>

TEST(itch, test1)
{
    const uint8_t arr[] = { 'S', 0x01, 0x00, 0x02, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 'O'};
    itch::system_event msg(arr);
    const uint8_t* record = msg.get_record();
    for(int i = 0; i < 12; ++i)
        EXPECT_EQ(arr[i], record[i]);
    EXPECT_EQ(msg.get_raw_byte(0), 'S');
    EXPECT_EQ(msg.get_raw_byte(1), 0x01);
    EXPECT_EQ(msg.get_raw_byte(2), 0x00);
    EXPECT_EQ(msg.get_raw_byte(3), 0x02);
}

TEST(itch, construct)
{
    itch::system_event msg;
    for(int i = 1; i < itch::SYSTEM_EVENT_LEN; ++i)
        EXPECT_EQ(msg.get_raw_byte(i), 0);
    msg.set_int(msg.STOCK_LOCATE, 1);
    msg.set_int(msg.TRACKING_NUMBER, 2);
    msg.set_int(msg.TIMESTAMP, 6);
    msg.set_string(msg.EVENT_CODE, "O");
    const uint8_t* record = msg.get_record();
    const uint8_t arr[] = { 'S', 0x00, 0x01, 0x00, 0x02, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 'O'};
    EXPECT_EQ(arr[0], record[0]);
    EXPECT_EQ(arr[1], record[1]);
    EXPECT_EQ(arr[2], record[2]);
    for(int i = 0; i < 12; ++i)
        EXPECT_EQ(arr[i], record[i]);
    EXPECT_EQ(msg.get_int(msg.STOCK_LOCATE), 1);
    EXPECT_EQ(msg.get_string(msg.EVENT_CODE), "O");
    EXPECT_EQ(msg.get_raw_byte(1), 0x00);
    EXPECT_EQ(msg.get_raw_byte(2), 0x01);
}

TEST(itch, largeNumbers)
{
    /*
    POS Field
    0   Message Type
    1   Stock Locate
    3   Tracking number
    5   Timestamp
    11  Order reference number
    19  BUY_SELL_INDICATOR
    20  Shares
    24  Stock
    32  Price (4 bytes)
    */
    itch::add_order msg;
    int32_t price1 = 1000;
    msg.set_int(itch::add_order::PRICE, price1);
    int sz = msg.get_size();
    const uint8_t* record = msg.get_record();
    int32_t price2 = msg.get_int(itch::add_order::PRICE);
    EXPECT_EQ(price1, price2);
}