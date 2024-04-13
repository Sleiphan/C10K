#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <pthread.h>

#include <sys/epoll.h>
#include <sys/eventfd.h>



int ep_instance;
eventfd_t signaler;



struct epoll_event create_epoll_event(int fd, uint32_t events) {
    epoll_data_t d;
    d.fd = fd;

    struct epoll_event event;
    event.events = events; // What events to listen to
    event.data = d;

    return event;
}



void* side_proc(void* args) {
    int event_count = 0;
    struct epoll_event event;

    while (event_count == 0)
        event_count = epoll_wait(ep_instance, &event, 1, 10000);

    const int err = errno;
    printf("Errno = '%s'\n", strerror(err));
    
    return NULL;
}



int main() {
    ep_instance = epoll_create1(0);

    printf("This test should end immediately, lasting less than 2 seconds\n");
    time_t start = time(NULL);

    pthread_t thread;
    pthread_create(&thread, NULL, side_proc, NULL);

    // Add an event signaler to flush all the waiting threads
    signaler = eventfd(2, 0);
    struct epoll_event signaler_events = create_epoll_event(signaler, EPOLLIN);
    epoll_ctl(ep_instance, EPOLL_CTL_ADD, signaler, &signaler_events);
    pthread_join(thread, NULL);

    time_t diff = time(NULL) - start;
    printf("The test lasted %zds\n", diff);


    epoll_ctl(ep_instance, EPOLL_CTL_DEL, signaler, &signaler_events);
    close(signaler);
    close(ep_instance);

    return 0;
}