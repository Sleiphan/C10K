#include <gtest/gtest.h>

extern "C" {
    #include <unix/queue.h>
}

TEST(queue, basic_function) {
    int numbers[] = { 0, 1, 2, 3, 4, 5, 6 };

    queue q = q_create();

    for (int& i : numbers)
        q_push(&q, &i);
    
    for (int i : numbers)
        EXPECT_EQ(i, *((int*) q_next(&q)));
    
    q_destroy(q);
}

TEST(queue, push_requiring_resize) {
    int numbers[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    queue q = q_create();

    for (int& i : numbers)
        q_push(&q, &i);
    
    for (int i : numbers)
        EXPECT_EQ(i, *((int*) q_next(&q)));
    
    q_destroy(q);
}

TEST(queue, circle_test) {
    int numbers[] = { 0, 1, 2, 3, 4, 5, 6, 7};

    queue q = q_create();

    long capacity = q._range_mask + 1;

    for (int i = 0; i < capacity / 2; i++)
        q_push(&q, &numbers[i]);
    
    for (int i = 0; i < capacity / 2; i++)
        EXPECT_EQ(i, *((int*) q_next(&q)));
    
    for (int i = 0; i < capacity; i++)
        q_push(&q, &numbers[i]);
    
    for (int i = 0; i < capacity; i++)
        EXPECT_EQ(i, *((int*) q_next(&q)));
    
    q_destroy(q);
}

TEST(queue, giant_data_set) {
    const long count = 10'000'000;

    int* numbers = new int[count];
    for (long i = 0; i < count; i++)
        numbers[i] = rand();
    

    queue q = q_create();

    for (long i = 0; i < count; i++)
        q_push(&q, &numbers[i]);
    
    for (long i = 0; i < count; i++)
        EXPECT_EQ(numbers[i], *((int*) q_next(&q)));
    
    q_destroy(q);

    delete[] numbers;
}

TEST(queue, is_empty) {
    queue q = q_create();

    EXPECT_TRUE(q_is_empty(q));

    q_push(&q, (void*) 1);

    EXPECT_FALSE(q_is_empty(q));

    q_next(&q);
    
    EXPECT_TRUE(q_is_empty(q));
    
    q_destroy(q);
}