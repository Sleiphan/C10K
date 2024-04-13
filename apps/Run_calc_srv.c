#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <unistd.h>

#include <fcntl.h>

#include <pthread.h>

#include "unix/endpoint.h"
#include "unix/workers.h"



endpoint e;



void send_welcome(int client_fd) {
    
}

void send_guide(int client_fd) {
    
}

void calc_request(int client_fd) {
    static const int REQ_SIZE_MAX = 128;
    char buffer[REQ_SIZE_MAX];

    int bytes_received = recv(client_fd, buffer, REQ_SIZE_MAX, 0);

    if (bytes_received >= 127) {
        sprintf(buffer, "Request too long. Goodbye\n");
        send(client_fd, buffer, strlen(buffer), 0);
        endpoint_disconnect_client(&e, client_fd);
        return;
    }

    if (bytes_received <= 0)
        return;
    

    if (buffer[0] != 'c') {
        send_guide(client_fd);
        sprintf(buffer, "Unknown command: %c\n", buffer[0]);
        send(client_fd, buffer, strlen(buffer), 0);
        return;
    }

    buffer[bytes_received] = '\0';

    char command;
    char num_buf_1[REQ_SIZE_MAX];
    char operator;
    char num_buf_2[REQ_SIZE_MAX];

    int sscanf_matches = sscanf(buffer, "%c %s %c %s", &command, num_buf_1, &operator, num_buf_2);
    if (sscanf_matches != 4) {
        send_guide(client_fd);
        sprintf(buffer, "Failed to read input format.\n");
        send(client_fd, buffer, strlen(buffer), 0);
        return;
    }

    int number_1 = atoi(num_buf_1);
    int number_2 = atoi(num_buf_2);
    int result = 0;

    switch (operator) {
        case '+': result = number_1 + number_2; break;
        case '-': result = number_1 - number_2; break;
        case '*': result = number_1 * number_2; break;
        case '/': result = number_1 / number_2; break;
    }

    sprintf(buffer, "%d %c %d = %d\n", number_1, operator, number_2, result);
    send(client_fd, buffer, strlen(buffer), 0);
}



bool stop = false;

void* accept_proc(void* args) {
    endpoint* e = (endpoint*) args;

    const uint32_t flags = EPOLLHUP | EPOLLIN | EPOLLET;

    endpoint_close_hungups(e);
    
    int new_client = -1;
    int count = endpoint_accept(e, &new_client, 1, 1000, flags);

    while (count > -1 && !stop) {
        if (count > 0) {
            printf("Socket connected, with fd=%d\n", new_client);
            // calc_srv_new_client(&server, new_client);
        }

        count = endpoint_accept(e, &new_client, 1, 1000, flags);
    }

    return NULL;
}



void client_event(void* args) {
    int client = (intptr_t) args;
    int event_flag = ((int64_t)args >> 32);

    if (event_flag & ~(EPOLLIN | EPOLLRDHUP))
        printf("Socket fd=%d event=%d\n", client, event_flag);

    if (event_flag & EPOLLIN) {
        calc_request(client);
    }

    if (event_flag & EPOLLRDHUP) {
        printf("Socket fd=%d disconnected\n", client);
    }
    
}



typedef struct client_event_processor_arg {
    endpoint* e;
    workers* w;
} client_event_processor_arg;

void* client_event_processor(void* args) {
    client_event_processor_arg* c_args = (client_event_processor_arg*) args;

    endpoint* e = c_args->e;
    workers*  w = c_args->w;

    int client = -1;
    int event_flag = -1;
    int count = endpoint_wait_event(e, &client, &event_flag, 1, 1000);

    while (count > -1 && !stop) {
        int64_t arg = (int64_t)client | ((int64_t)event_flag << 32);
        if (count > 0)
            w_post(w, client_event, (void*) (intptr_t) arg);
            // client_event(arg);

        count = endpoint_wait_event(e, &client, &event_flag, 1, 1000);
    }

    return NULL;
}





int main() {
    workers worker_pool;
    w_create(&worker_pool, 5);

    endpoint_create(&e, 7999, false);

    client_event_processor_arg c_args;
    c_args.e = &e;
    c_args.w = &worker_pool;

    pthread_t t_accept;
    pthread_t t_event;
    
    pthread_create(&t_accept, NULL, accept_proc, &e);
    pthread_create(&t_event,  NULL, client_event_processor, &c_args);

    printf("Server now running. Enter anything to stop it.\n");
    char in_buf[64];
    scanf("%s", in_buf);

    stop = true;
    pthread_join(t_accept, NULL);
    pthread_join(t_event, NULL);
    w_join(&worker_pool);

    endpoint_close(&e);

    return 0;
}
