#ifndef CUSTOM_UNIX_WORKER_POOL
#define CUSTOM_UNIX_WORKER_POOL

#include <pthread.h>
#include "unix/queue.h"



typedef struct {
    int _num_threads;
    pthread_t* _threads;
    int _stop;

    queue _jobs;
    pthread_mutex_t _lock;
    pthread_cond_t _cv;
} workers;

void w_create(workers* workers, int num_threads);

void w_post(workers* workers, void(*job)(void* args), void* args);

void w_join(workers*);



#endif // CUSTOM_UNIX_WORKER_POOL