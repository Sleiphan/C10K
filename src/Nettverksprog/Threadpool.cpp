#include "Nettverksprog/Threadpool.hpp"



Threadpool::Threadpool(int num_threads) : num_threads(num_threads) {
    threads = new std::thread[num_threads];

    while (num_threads--)
        threads[num_threads] = std::thread(&Threadpool::process, this);
}

Threadpool::~Threadpool() {
    join();
}



void Threadpool::post(void(*function_p)()) {
    std::lock_guard<std::mutex> lock(jobs_lock);
    jobs.push(function_p);
    cv.notify_one();
}



void Threadpool::process() {
    while (!jobs.empty() || !stop) {
        std::unique_lock<std::mutex> lock(jobs_lock);

        // Wait until a job appears, or the we are supposed to stop
        while (jobs.empty() && !stop)
            cv.wait(lock);

        // Skip job-performing if we are shutting down
        if (jobs.empty() && stop) {
            lock.unlock();
            cv.notify_one();
            break;
        }
        
        // Pluck a job from the queue
        void(*function_p)() = jobs.front();

        // Remove the job from the queue
        jobs.pop();

        // Wake up the other threads that are waiting
        lock.unlock();
        cv.notify_one();

        // Do the job
        function_p();
    }
}

void Threadpool::join() {
    if (stop)
        return;
    
    stop = true;
    cv.notify_one();
    
    for (std::thread* t = threads; t != threads + num_threads; t++)
        t->join();
    
    delete[] threads;
}