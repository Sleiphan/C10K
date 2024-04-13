
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include <pthread.h>

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <fcntl.h>

#include <arpa/inet.h>

#include <sys/ioctl.h>
#include <sys/socket.h>




int ep_instance;
eventfd_t signaler;
int srv_fd;



struct epoll_event create_epoll_event_test(int fd, uint32_t events) {
    epoll_data_t d;
    d.fd = fd;

    struct epoll_event event;
    event.events = events; // What events to listen to
    event.data = d;

    return event;
}



void flush_and_close(int socket_fd) {
    shutdown(socket_fd, SHUT_WR);

    // unsigned long pending = 0;
    // int err = 0;
    // do {
        // err = ioctl(socket_fd, SIOCOUTQ, &pending);
    // } while(pending > 0);

    close(socket_fd);
}



void* side_proc(void* args) {
    int event_count = 0;
    struct epoll_event event;

    while (1) {
        event_count = epoll_wait(ep_instance, &event, 1, 1000);

        if (event_count == 0)
            continue;

        if (event_count == -1 && errno == EINTR)
            continue;

        if (event.data.fd == signaler)
            break;
        
        // We have waiting clients!

        struct sockaddr client_address;
        socklen_t client_address_len;
        int new_socket;

        while ((new_socket = accept(srv_fd, &client_address, &client_address_len)) != -1) {
            // Write a small message to the client and remove them
            char msg[] = "Connection established <3 Goodbye!\n";
            send(new_socket, msg, strlen(msg), 0);
            flush_and_close(new_socket);
        }
    }
    
    return NULL;
}



int create_server_socket(int PORT_NUMBER) {
    int srv_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Make the socket non-blocking
    fcntl(srv_fd, F_SETFL, fcntl(srv_fd, F_GETFL, 0) | O_NONBLOCK);

    // The address to bind the socket to
    struct sockaddr_in serverAddr;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_NUMBER);

    // Bind the socket
    bind(srv_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    // Make the socket a server-socket
    listen(srv_fd, 512);

    return srv_fd;
}



int main() {
    ep_instance = epoll_create1(0);



    // Setup server socket
    srv_fd = create_server_socket(7999);
    struct epoll_event srv_events = create_epoll_event_test(srv_fd, EPOLLIN | EPOLLET | EPOLLWAKEUP);
    epoll_ctl(ep_instance, EPOLL_CTL_ADD, srv_fd, &srv_events);



    // Setup the signaler before entering the loop
    signaler = eventfd(1, 0);


    
    pthread_t thread;
    pthread_create(&thread, NULL, side_proc, NULL);



    // Wait for ANYTHING to be entered by the user
    printf("Server running! Enter literally anything to make it stop.\n");
    char msg[] = "STOP";
    fgets(msg, strlen(msg), stdin);


    // Add an event signaler to flush all the waiting threads
    struct epoll_event signaler_events = create_epoll_event_test(signaler, EPOLLIN);
    epoll_ctl(ep_instance, EPOLL_CTL_ADD, signaler, &signaler_events); // Send the signal by adding the signaler
    pthread_join(thread, NULL);
    close(signaler);




    close(ep_instance);
    return 0;
}