#include <gtest/gtest.h>

extern "C" {
    #include "unix/boolset.h"
}



TEST(boolset, one_billion_bits_with_a_few_bits_set_to_1) {
    boolset_t b = boolset_create();

    for (int i = 1; i < 1'000'000'000; i += i)
        boolset_set(&b, i, 1);
        
    
    int poll = 1;
    for (int i = 0; i < b.size * sizeof(BOOLSET_TYPE); i++) {
        const int expected = i == poll;
        const int actual = boolset_get(&b, i);

        ASSERT_EQ(actual, expected);

        poll += poll * expected;
    }

    boolset_destroy(&b);
}



TEST(boolset, the_first_bit) {
    boolset_t b = boolset_create();

    int actual = boolset_get(&b, 0);
    ASSERT_EQ(actual, 0);
    
    boolset_set(&b, 0, 0);
    actual = boolset_get(&b, 0);
    ASSERT_EQ(actual, 0);
    
    boolset_set(&b, 0, 1);
    actual = boolset_get(&b, 0);
    ASSERT_EQ(actual, 1);
    
    boolset_set(&b, 0, 0);
    actual = boolset_get(&b, 0);
    ASSERT_EQ(actual, 0);

    actual = boolset_get(&b, 0);
    ASSERT_EQ(actual, 0);

    boolset_destroy(&b);
}





void for_each_test_cb(BOOLSET_TYPE index, void* args) {
    int* val = (int*) args;

    ASSERT_EQ(*val, index);

    *val += *val;
}

TEST(boolset, for_each) {
    boolset_t b = boolset_create();

    for (int i = 1; i < 1'000'000'000; i += i)
        boolset_set(&b, i, 1);
    
    int for_each_test_poll = 1;
    
    boolset_foreach(&b, for_each_test_cb, &for_each_test_poll);

    boolset_destroy(&b);
}