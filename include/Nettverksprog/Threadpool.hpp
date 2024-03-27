#ifndef IDATT2104_NETTVERKSPROGRAMMERING_THREADPOOL
#define IDATT2104_NETTVERKSPROGRAMMERING_THREADPOOL



#include <queue>
#include <thread>
#include <condition_variable>
#include <mutex>

class Threadpool {
private:
    typedef struct {
        
    } job;

    bool stop = false;

    std::thread* threads;
    const int num_threads;

    std::queue<void(*)()> jobs;

    std::mutex jobs_lock;
    std::condition_variable cv;

    void process();
    
public:
    Threadpool(int thread_count);
    ~Threadpool();

    void post(void(*function_p)());

    void join();
};



#endif // IDATT2104_NETTVERKSPROGRAMMERING_THREADPOOL