#include "ouch.h"
#include <gtest/gtest.h>

TEST(ouch, test1)
{
    ouch::enter_order msg;
    msg.set_string(msg.USER_REF_NUM, "ABC1");
    msg.set_string(msg.SIDE, "B");
    EXPECT_EQ(msg.get_string(msg.USER_REF_NUM), "ABC1");
    EXPECT_EQ(msg.get_string(msg.SIDE), "B");
    EXPECT_EQ(msg.get_int(msg.APPENDAGE_LENGTH), 0);
    EXPECT_EQ(msg.get_tag_values(), nullptr);
    // add a tag value (appendage)
    msg.add_tag_value(ouch::tag_record::tag_name::FIRM, "ABCD");
    EXPECT_EQ(msg.get_int(msg.APPENDAGE_LENGTH), 6); // 2 for the tag_value + 4 for ABCD
    char expected[] = { 0x05, 0x02, 'A', 'B', 'C', 'D'};
    const char* retrieved = msg.get_tag_values();
    EXPECT_EQ(expected[0], retrieved[0]);
    EXPECT_EQ(expected[1], retrieved[1]);
    EXPECT_EQ(expected[2], retrieved[2]);
    EXPECT_EQ(expected[3], retrieved[3]);
    EXPECT_EQ(expected[4], retrieved[4]);
    EXPECT_EQ(expected[5], retrieved[5]);
    EXPECT_EQ(msg.get_tag_value_string(ouch::tag_record::tag_name::FIRM), "ABCD");
    msg.add_tag_value(ouch::tag_record::tag_name::MIN_QTY, 100);
    char expected2[] = { 0x05, 0x02, 'A', 'B', 'C', 'D', 0x05, 0x03, 0x00, 0x00, 0x00, 0x64};
    retrieved = msg.get_tag_values();
    EXPECT_EQ(expected2[0], retrieved[0]);
    EXPECT_EQ(expected2[1], retrieved[1]);
    EXPECT_EQ(expected2[2], retrieved[2]);
    EXPECT_EQ(expected2[3], retrieved[3]);
    EXPECT_EQ(expected2[4], retrieved[4]);
    EXPECT_EQ(expected2[5], retrieved[5]);
    EXPECT_EQ(expected2[6], retrieved[6]);
    EXPECT_EQ(expected2[7], retrieved[7]);
    EXPECT_EQ(expected2[8], retrieved[8]);
    EXPECT_EQ(expected2[9], retrieved[9]);
    EXPECT_EQ(expected2[10], retrieved[10]);
    EXPECT_EQ(expected2[11], retrieved[11]);
    EXPECT_EQ(msg.get_tag_value_int(ouch::tag_record::tag_name::MIN_QTY), 100);
}

