#include <unix/workers.h>
#include <stdlib.h>



typedef struct {
    void* (*fn)(void*);
    void* args;
    void** return_location;
} worker_job;



void* w_process(void* arg) {
    workers* w = (workers*) arg;
    
    while (!q_is_empty(w->_jobs) || !w->_stop) {
        pthread_mutex_lock(&w->_lock);

        // Wait until a job appears, or that we are supposed to stop
        while (q_is_empty(w->_jobs) && !w->_stop)
            pthread_cond_wait(&w->_cv, &w->_lock);

        // Skip job-performing if we are shutting down
        if (q_is_empty(w->_jobs) && w->_stop) {
            pthread_mutex_unlock(&w->_lock);
            pthread_cond_signal(&w->_cv);
            break;
        }
        
        // Pluck a job from the queue
        void*(*function_p)(void*) = q_next(&w->_jobs);
        void* args = q_next(&w->_jobs);

        // Wake up the other threads that are waiting
        pthread_mutex_unlock(&w->_lock);
        pthread_cond_signal(&w->_cv);

        // Do the job
        function_p(args);
    }
}



void w_create(workers* w, int num_threads) {
    w->_num_threads = num_threads;
    w->_stop = 0;

    w->_jobs = q_create();
    pthread_mutex_init(&w->_lock, NULL);
    pthread_cond_init(&w->_cv, NULL);

    w->_threads = (pthread_t*) malloc(num_threads * sizeof(pthread_t));
    for (int i = 0; i < w->_num_threads; i++)
        pthread_create(&w->_threads[i], NULL, w_process, w);
}



void w_post(workers* w, void(*job)(void* args), void* args) {
    pthread_mutex_lock(&w->_lock);
    q_push(&w->_jobs, job);
    q_push(&w->_jobs, args);
    pthread_mutex_unlock(&w->_lock);
    
    pthread_cond_signal(&w->_cv);
}



void w_join(workers* w) {
    if (w->_stop)
        return;
    w->_stop = 1;

    pthread_cond_signal(&w->_cv);

    for (int i = 0; i < w->_num_threads; i++)
        pthread_join(w->_threads[i], NULL);
    
    free(w->_threads);
}

