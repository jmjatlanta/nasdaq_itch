#include "ouch.h"
#include <gtest/gtest.h>

TEST(ouch, test1)
{
    ouch::enter_order msg;
    msg.set_string(ouch::enter_order::USER_REF_NUM, "ABC1");
    msg.set_string(ouch::enter_order::SIDE, "B");
    EXPECT_EQ(msg.get_string(ouch::enter_order::USER_REF_NUM), "ABC1");
    EXPECT_EQ(msg.get_string(ouch::enter_order::SIDE), "B");
    EXPECT_EQ(msg.get_int(ouch::enter_order::APPENDAGE_LENGTH), 0);
    EXPECT_EQ(msg.get_appendages(), nullptr);
}

