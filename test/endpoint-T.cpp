#include <gtest/gtest.h>

#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/socket.h>

extern "C" {
    #include "unix/endpoint.h"
}



int create_client_socket(const char* host, const int port, int udp, int IPv6) {
    const int client_fd = socket(AF_INET, udp ? SOCK_DGRAM : SOCK_STREAM, 0);

    const int ip_version = IPv6 ? AF_INET6 : AF_INET;

    // The address to bind the socket to
    struct sockaddr_in server_address;
    inet_pton(ip_version, host, &server_address.sin_addr);
    server_address.sin_family = ip_version;
    server_address.sin_port = htons(port);

    int err = connect(client_fd, (sockaddr*) &server_address, sizeof(server_address));

    return client_fd;
}

void* accept_proc_1(void* args) {
    endpoint* e = (endpoint*) args;

    int new_client = -1;

    while (endpoint_accept(e, &new_client, 1, 500, EPOLLIN) == 0);

    return NULL;
}

TEST(endpoint, can_read_whole_message_when_client_disconnects) {
    static const int PORT_NUMBER = 7999;
    static const int BUFFER_SIZE = 1 << 16;

    endpoint e;
    endpoint_create(&e, PORT_NUMBER, false);

    pthread_t t_accept;
    pthread_create(&t_accept, NULL, accept_proc_1, &e);

    char in_buf[BUFFER_SIZE];
    char out_buf[BUFFER_SIZE];

    static const int char_mask = (1 << 8) - 1;
    for (int i = 0; i < BUFFER_SIZE - 1; ++i)
        out_buf[i] = (rand() & char_mask) + 1;
    
    for (int i = 0; i < BUFFER_SIZE - 1; ++i)
        out_buf[i] += out_buf[i] == 0;
    
    out_buf[BUFFER_SIZE - 1] = 0;
    

    int client_fd = create_client_socket("127.0.0.1", PORT_NUMBER, 0, 0);
    send(client_fd, out_buf, BUFFER_SIZE, 0);
    shutdown(client_fd, SHUT_WR);
    close(client_fd);

    int event_flags = 0;
    while (endpoint_wait_event(&e, &client_fd, &event_flags, 1, 500) == 0);
    
    int bytes_received = recv(client_fd, in_buf, BUFFER_SIZE, 0);

    pthread_join(t_accept, NULL);

    endpoint_disconnect_all_clients(&e);

    EXPECT_STREQ(in_buf, out_buf);
}



TEST(endpoint, starts_and_stops_correctly) {
    endpoint e;
    endpoint_create(&e, 7999, false);

    endpoint_close(&e);
}



