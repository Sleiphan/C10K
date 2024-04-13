#include <stdio.h>
#include <stdbool.h>
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
#include "unix/http.h"





void html_answer(int fd) {
    const size_t BUFFER_SIZE = 4096;
    char in_buf[BUFFER_SIZE];
    char out_buf[BUFFER_SIZE];

    static const int HEADER_BUFFER_SIZE = 48;
    http_header header_buffer[HEADER_BUFFER_SIZE];

    ssize_t bytes_read = recv(fd, in_buf, BUFFER_SIZE - 1, 0);
    in_buf[bytes_read] = '\0';

    if (bytes_read <= 0)
        return;
    
    if (in_buf[5] != ' ') {
        strcpy(out_buf, "HTTP/1.1 404 Not Found\r\nConnection: close");
        send(fd, out_buf, strlen(out_buf), 0);
        return;
    }

    printf("Sending html answer to fd=%d\n", fd);



    int header_start = 0;
    while (in_buf[header_start] != '\r')
        ++header_start;
    header_start += 2;

    int header_end = header_start;
    while (header_end < bytes_read) { // Scan to the end of the headers
        while (in_buf[header_end] != '\r' && header_end < bytes_read)
            ++header_end;

        int found = 1;
        found &= in_buf[header_end + 1] == '\n';
        found &= in_buf[header_end + 2] == '\r';
        found &= in_buf[header_end + 3] == '\n';

        if (found)
            break;
        else
            header_end += 2;
    }

    int header_count = http_str_to_headers(header_buffer, HEADER_BUFFER_SIZE, &in_buf[header_start], header_end - header_start);
    memset(in_buf, 0, BUFFER_SIZE);
    
    for (int i = 0; i < header_count; i++) {
        strcpy(out_buf, "<li>");
        http_header_to_str(&out_buf[4], header_buffer[i], BUFFER_SIZE - 4);
        strncat(out_buf, "</li>", 6);

        // Concat the header into the buffer
        strncat(in_buf, out_buf, strlen(out_buf));
    }

    const char html_page_begin[] = "<!DOCTYPE html><html><h1>Welcome!</h1><p>These are the headers you sent me:</p><ul>";
    const char html_page_end[] = "</ul></html>";

    size_t html_page_begin_len = strlen(html_page_begin);
    size_t html_page_headers_len = strlen(in_buf);
    size_t html_page_end_len = strlen(html_page_end);
    
    char response[128];
    sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %zd\r\nConnection: close\r\n\r\n",
        html_page_begin_len + html_page_headers_len + html_page_end_len - 1);

    strcpy(out_buf, response);
    strncat(out_buf, html_page_begin, strlen(html_page_begin));
    strncat(out_buf, in_buf, strlen(in_buf));
    strncat(out_buf, html_page_end, strlen(html_page_end));

    // printf("%s\n", out_buf);

    send(fd, out_buf, strlen(out_buf), 0);
}





bool stop = false;

void* accept_proc(void* args) {
    endpoint* e = (endpoint*) args;

    const uint32_t flags = EPOLLHUP | EPOLLIN | EPOLLET;
    
    int new_client = -1;
    int count = endpoint_accept(e, &new_client, 1, 1000, flags);

    while (count > -1 && !stop) {
        if (count > 0)
            printf("Socket connected, with fd=%d\n", new_client);
            
        endpoint_close_hungups(e);
        count = endpoint_accept(e, &new_client, 1, 1000, flags);
    }

    return NULL;
}



void client_event(void* args) {
    int client = (intptr_t) args;
    int event_flag = ((int64_t)args >> 32);

    if (event_flag & ~(EPOLLIN | EPOLLRDHUP))
        printf("Socket fd=%d event=%d\n", client, event_flag);

    if (event_flag & EPOLLIN)
        html_answer(client);

    if (event_flag & EPOLLRDHUP)
        printf("Socket fd=%d disconnected\n", client);

    
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





void print_epoll_event_flag_values() {
    printf("EPOLLIN        = %d\n", EPOLLIN);
    printf("EPOLLOUT       = %d\n", EPOLLOUT);
    printf("EPOLLRDHUP     = %d\n", EPOLLRDHUP);
    printf("EPOLLPRI       = %d\n", EPOLLPRI);
    printf("EPOLLERR       = %d\n", EPOLLERR);
    printf("EPOLLHUP       = %d\n", EPOLLHUP);
    printf("EPOLLET        = %d\n", EPOLLIN);
    printf("EPOLLONESHOT   = %d\n", EPOLLIN);
    printf("EPOLLWAKEUP    = %d\n", EPOLLIN);
    printf("EPOLLEXCLUSIVE = %d\n", EPOLLIN);
}





int main() {
    // print_epoll_event_flag_values();

    workers worker_pool;
    w_create(&worker_pool, 5);

    endpoint e;
    endpoint_create(&e, 7999, false);

    client_event_processor_arg c_args;
    c_args.e = &e;
    c_args.w = &worker_pool;

    pthread_t t_accept;
    pthread_t t_event;
    
    pthread_create(&t_accept, NULL, accept_proc, &e);
    pthread_create(&t_event,  NULL, client_event_processor, &c_args);

    char in_buf[64];
    scanf("%s", in_buf);

    stop = true;
    pthread_join(t_accept, NULL);
    pthread_join(t_event, NULL);
    w_join(&worker_pool);

    endpoint_close(&e);

    return 0;
}
