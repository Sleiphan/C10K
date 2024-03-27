#include <gtest/gtest.h>


TEST(Example_library, sum) {
    int i = 17;

    EXPECT_EQ(i, 17);

    // EXAMPLE: Expect two strings not to be equal.
    EXPECT_STRNE("hello", "world");
}