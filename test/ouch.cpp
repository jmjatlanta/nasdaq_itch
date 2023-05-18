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
    EXPECT_EQ(msg.get_appendages(), nullptr);
}

