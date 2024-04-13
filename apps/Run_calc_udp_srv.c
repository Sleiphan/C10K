#include<stdbool.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<stdint.h>
#include<unistd.h>
#include<pthread.h>

#include<arpa/inet.h>
#include<sys/socket.h>



#define PORT 7999 // The port to listen to for incoming data
#define MAX_UDP_PAYLOAD_SIZE 508 // The maximum safe UDP payload is 508 bytes



int create_server_socket_udp(int PORT_NUMBER, bool IPv6) {
    static const int type  = SOCK_DGRAM;
    static const int proto = IPPROTO_UDP;
    const int domain = IPv6 ? AF_INET6 : AF_INET;

    struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int socket_fd = socket(domain, type, proto);
    bind(socket_fd , (struct sockaddr*) &server_addr, sizeof(server_addr));

    return socket_fd;
}


void send_guide(int socket_fd) {

}


int calc_request(int udp_socket_fd) {
    static const int REQ_SIZE_MAX = 128;
    char buffer[MAX_UDP_PAYLOAD_SIZE];

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int bytes_received = recvfrom(udp_socket_fd, buffer, MAX_UDP_PAYLOAD_SIZE, 0, (struct sockaddr*) &client_addr, &client_addr_len);

    if (bytes_received == -1)
        return -1;

    if (bytes_received >= 127) {
        sprintf(buffer, "Request too long. Goodbye\n");
        sendto(udp_socket_fd, buffer, strlen(buffer), 0, (struct sockaddr*) &client_addr, client_addr_len);
        return 0;
    }

    if (bytes_received <= 0)
        return 0;
    

    if (buffer[0] != 'c') {
        send_guide(udp_socket_fd);
        sprintf(buffer, "Unknown command: %c\n", buffer[0]);
        sendto(udp_socket_fd, buffer, strlen(buffer), 0, (struct sockaddr*) &client_addr, client_addr_len);
        return 0;
    }

    buffer[bytes_received] = '\0';

    char command;
    char num_buf_1[REQ_SIZE_MAX];
    char operator;
    char num_buf_2[REQ_SIZE_MAX];

    int sscanf_matches = sscanf(buffer, "%c %s %c %s", &command, num_buf_1, &operator, num_buf_2);
    if (sscanf_matches != 4) {
        send_guide(udp_socket_fd);
        sprintf(buffer, "Failed to read input format.\n");
        sendto(udp_socket_fd, buffer, strlen(buffer), 0, (struct sockaddr*) &client_addr, client_addr_len);
        return 0;
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
    sendto(udp_socket_fd, buffer, strlen(buffer), 0, (struct sockaddr*) &client_addr, client_addr_len);

    return 0;
}



void* srv_proc(void* args) {
	char in_buf[MAX_UDP_PAYLOAD_SIZE];

    int socket_fd = (intptr_t) args;
    
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    while (calc_request(socket_fd) != -1);

    // int bytes_received = recvfrom(socket_fd, in_buf, MAX_UDP_PAYLOAD_SIZE, 0, (struct sockaddr*) &client_addr, &client_addr_len);

    // while (bytes_received != -1) {
    //     int err = sendto(socket_fd, in_buf, bytes_received, 0, (struct sockaddr*) &client_addr, client_addr_len);
    //     if (err == -1)
    //         perror("Error answering client");

    //     bytes_received = recvfrom(socket_fd, in_buf, MAX_UDP_PAYLOAD_SIZE, 0, (struct sockaddr*) &client_addr, &client_addr_len);
    // }

    return NULL;
}



int main(void) {
    int server_fd = create_server_socket_udp(PORT, false);

    pthread_t t_server;
    pthread_create(&t_server, NULL, srv_proc, (void*) (intptr_t) server_fd);

    printf("Server is running. Enter anything to close the server\n");
    char in_buf[64];
    scanf("%s", in_buf);

	close(server_fd);
    // pthread_join(t_server, NULL);

	return 0;
}