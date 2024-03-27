#include <gtest/gtest.h>
#include <mutex>

extern "C" {
    #include "unix/workers.h"
}


TEST(workers, shuts_down_after_initial_wait) {
    workers w;
    w_create(&w, 5);

    sleep(2);

    w_join(&w);
}



void shuts_down_after_wait_proc(void*) {
    int i = 0;

    while (i <= 10) i++;
}

TEST(workers, shuts_down_after_wait) {
    workers w;
    w_create(&w, 5);

    for (int i = 0; i < 10; i++)
        w_post(&w, shuts_down_after_wait_proc, NULL);

    sleep(2);

    w_join(&w);
}





int finishes_all_jobs_counter = 0;

void finishes_all_jobs_proc(void*) {
    static std::mutex finishes_all_jobs_mtx;

    sleep(1);
    finishes_all_jobs_mtx.lock();
    finishes_all_jobs_counter += 1;
    finishes_all_jobs_mtx.unlock();
}

TEST(workers, finishes_all_jobs) {
    finishes_all_jobs_counter = 0;

    const int NUM_JOBS = 6;

    workers w;
    w_create(&w, 3);

    for (int i = 0; i < NUM_JOBS; i++)
        w_post(&w, finishes_all_jobs_proc, NULL);
    
    w_join(&w);

    EXPECT_EQ(finishes_all_jobs_counter, NUM_JOBS);
}



void handles_arguments_proc(void* arg) {
    int* i = (int*) arg;
    *i += 1;
}

TEST(workers, handles_arguments) {
    const int NUM_JOBS = 6;
    int* arg_vals = new int[NUM_JOBS];

    workers w;
    w_create(&w, NUM_JOBS / 2);

    for (int i = 0; i < NUM_JOBS; i++) {
        arg_vals[i] = i;
        w_post(&w, handles_arguments_proc, &arg_vals[i]);
    }
    
    w_join(&w);

    for (int i = 0; i < NUM_JOBS; i++)
        EXPECT_EQ(arg_vals[i], i + 1);

    delete[] arg_vals;
}