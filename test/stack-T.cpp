#include <gtest/gtest.h>


extern "C" {
    #include "unix/stack.h"
}


TEST(stack, returns_two_values_corretly) {
    stack s = stack_create();

    void* val_1 = (void*) 1;
    void* val_2 = (void*) 2;

    stack_push(&s, val_1);
    stack_push(&s, val_2);

    EXPECT_EQ(val_2, stack_pop(&s));
    EXPECT_EQ(val_1, stack_pop(&s));

    stack_destroy(&s);
}

TEST(stack, push_test) {
    stack s = stack_create();
    stack_push(&s, (void*) 1);

    EXPECT_NE(s.top, (stack_node*) 0);
    EXPECT_EQ(s.top->data, (void*) 1);
    EXPECT_EQ(s.top->prev, (stack_node*) 0);

    stack_destroy(&s);
}

TEST(stack, has_valid_initial_values) {
    stack s = stack_create();
    
    EXPECT_EQ(s.top, (stack_node*) 0);

    stack_destroy(&s);
}

TEST(stack, empty_test) {
    stack s = stack_create();
    
    EXPECT_EQ(stack_is_empty(s), 1);
    stack_push(&s, (void*) 1);
    EXPECT_EQ(stack_is_empty(s), 0);
    stack_pop(&s);
    EXPECT_EQ(stack_is_empty(s), 1);

    stack_destroy(&s);
}

TEST(stack, destroy_test) {
    stack s = stack_create();
    stack_push(&s, (void*) 1);
    stack_push(&s, (void*) 2);
    stack_push(&s, (void*) 3);
    stack_push(&s, (void*) 4);

    stack_destroy(&s);

    EXPECT_EQ(s.top, (void*) 0);
}