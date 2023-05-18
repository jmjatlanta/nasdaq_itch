#include "itch.h"
#include <gtest/gtest.h>

TEST(simple, test1)
{
    const char arr[] = { 'S', 0x01, 0x00, 0x02, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 'O'};
    itch::system_event_message msg(arr);
    const char* record = msg.get_record();
    for(int i = 0; i < 12; ++i)
        EXPECT_EQ(arr[i], record[i]);
    EXPECT_EQ(msg.get_raw_byte(0), 'S');
    EXPECT_EQ(msg.get_raw_byte(1), 0x01);
    EXPECT_EQ(msg.get_raw_byte(2), 0x00);
    EXPECT_EQ(msg.get_raw_byte(3), 0x02);
}

TEST(simple, construct)
{
    itch::system_event_message msg;
    for(int i = 1; i < itch::SYSTEM_EVENT_LEN; ++i)
        EXPECT_EQ(msg.get_raw_byte(i), 0);
    msg.set_int(msg.STOCK_LOCATE, 1);
    msg.set_int(msg.TRACKING_NUMBER, 2);
    msg.set_int(msg.TIMESTAMP, 6);
    msg.set_string(msg.EVENT_CODE, "O");
    const char* record = msg.get_record();
    const char arr[] = { 'S', 0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 'O'};
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

