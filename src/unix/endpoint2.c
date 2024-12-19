#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/eventfd.h>

#include "unix/endpoint2.h"


// ### HELPER FUNCTIONS ###


void endpoint_register_client(endpoint2* e, int socket_fd) {
    pthread_mutex_lock(&e->_connected_clients_lock);
    boolset_set(&e->connected_clients_fds, socket_fd, 1);
    pthread_mutex_unlock(&e->_connected_clients_lock);
}

void endpoint_deregister_client(endpoint2* e, int socket_fd) {
    pthread_mutex_lock(&e->_connected_clients_lock);
    boolset_set(&e->connected_clients_fds, socket_fd, 0);
    pthread_mutex_unlock(&e->_connected_clients_lock);
}



// ### MORE HELPER FUNCTIONS ###

int create_server_socket(int PORT_NUMBER, bool IPv6) {
    const int type   = SOCK_STREAM;
    const int domain = IPv6 ? AF_INET6 : AF_INET;

    // Create the socket that is to become a server socket.
    // It will be a TCP socket, as we need the clients to
    // establish a unique connection with this endpoint.
    int srv_fd = socket(domain, type, IPPROTO_TCP);

    // Make the socket non-blocking
    fcntl(srv_fd, F_SETFL, fcntl(srv_fd, F_GETFL, 0) | O_NONBLOCK);

    // The address to bind the socket to
    struct sockaddr_in server_address;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_family = domain;
    server_address.sin_port = htons(PORT_NUMBER);

    // Bind the socket
    bind(srv_fd, (struct sockaddr*)&server_address, sizeof(server_address));

    // Make the socket a server-socket
    listen(srv_fd, 512);

    return srv_fd;
}

int close_hungups(endpoint2* e) {
    int count = 0;

    for (; !stack_is_empty(e->_closing_queue); ++count) {
        endpoint2_disconnect_client(e, (intptr_t) stack_pop(&e->_closing_queue));
    }

    return count;
}

void accept_new_clients_dontuse(endpoint2* e) {
    int new_client_count = 0; // Keeps count of how many clients we have accepted during this call to 'endpoint_accept'
    int new_client_fd = -1; // Receives the file descriptor of the client's socket
    struct sockaddr client_address; // Receives the client address
    socklen_t client_address_len; // Receives the length of the client address

    for (; new_client_count < new_fds_size && (new_client_fd = accept(e->_srv_fd, &client_address, &client_address_len)) != -1; ++new_client_count) {
        // Register the client in the boolset
        endpoint_register_client(e, new_client_fd);

        // Create event-object, making sure we allways listen to EPOLLRDHUP and that the clients are edge-triggered
        struct epoll_event client_event = {e->_epoll_event_whitelist | EPOLLRDHUP | EPOLLET, new_client_fd};

        // Tell the caller about the new connection
        new_fds[new_client_count] = new_client_fd;

        // Add client to the epoll-instance for client events
        epoll_ctl(e->_ep_selector, EPOLL_CTL_ADD, new_client_fd, &client_event);
    }

    return new_client_count;
}



// ### EXPORTED FUNCTIONS ###

void endpoint2_create(endpoint2* e, int port_number, bool IPv6) {
    // By default, no new clients are waiting to connect
    e->_connections_waiting = false;

    // Set the default events to listen for from client sockets
    e->_epoll_event_whitelist = EPOLLIN;

    // Create object for keeping record of connected clients
    e->connected_clients_fds = boolset_create();

    // Initialize mutex
    pthread_mutex_init(&e->_connected_clients_lock, NULL);

    // Create new epoll instance
    e->_epoll_instance = epoll_create1(0);

    // Create the server socket listening for new connections
    e->_server_fd = create_server_socket(port_number, IPv6);

    // These are the events the server socket should notify us about
    struct epoll_event srv_events = { EPOLLIN | EPOLLHUP | EPOLLET, e->_server_fd };

    // Add the server socket to the epoll instance
    epoll_ctl(e->_epoll_instance, EPOLL_CTL_ADD, e->_server_fd, &srv_events);
}



/*
void endpoint2_close(endpoint2* e) {
    close_hungups(e);
}
//*/



int endpoint2_step(endpoint2* e, struct epoll_event events[], int* max_events, const int timeout) {
    close_hungups(e);

    // Wait for events from epoll instance
    const int event_count = epoll_wait(e->_epoll_instance, events, *max_events, timeout);

    // Return if an error occurred
    if (event_count == -1)
        return -1;

    // The amount of events returned through the 'events' parameter
    int event_index = 0;

    // Run through the events
    for (; event_index < event_count; event_index++) {
        // Check whether the current event is for the server socket
        const bool is_server_socket = events[event_index].data.fd == e->_server_fd;

        // Signal awaiting clients
        e->_connections_waiting += is_server_socket;
        // Skip to next event if the index landed on the server socket
        event_index += is_server_socket;

        // Break off if that was the last event
        if (event_index >= event_count)
            break;
        
        // Perform disconnections
        if (events[event_index].events & EPOLLRDHUP)
            stack_push(&e->_closing_queue, events[event_index].data.fd);
        

    }
    
    // Next, accept new clients

    struct sockaddr client_address; // Receives the client address
    socklen_t client_address_len; // Receives the length of the client address

    for (int new_socket = -1; event_index < *max_events & ; event_index++) {
        new_socket = accept(e->_server_fd, &client_address, &client_address_len);

        if (new_socket)
            return -1;
    }

    *max_events = event_index + 1;

    return 0;
}



inline int endpoint2_disconnect_client(endpoint2* e, int socket_fd) {
    // Remove the client from the list of connected clients
    endpoint_deregister_client(e, socket_fd);

    // Remove the client from the selector epoll instance
    epoll_ctl(e->_epoll_instance, EPOLL_CTL_DEL, socket_fd, NULL);

    // Shut down both channels of communication
    shutdown(socket_fd, SHUT_RDWR);

    // Close the client's socket
    close(socket_fd);

    // Return success
    return 0;
}

void disconnect_all_foreach(BOOLSET_TYPE socket_fd, void* args) {
    endpoint2* e = (endpoint2*) args;

    boolset_set(&e->connected_clients_fds, socket_fd, 0);

    // Remove the client from the selector epoll instance
    epoll_ctl(e->_epoll_instance, EPOLL_CTL_DEL, socket_fd, NULL);

    // Shut down both channels of communication
    shutdown(socket_fd, SHUT_RDWR);

    // Close the client's socket
    close(socket_fd);
}

inline int endpoint_disconnect_all_clients(endpoint2* e) {
    pthread_mutex_lock(&e->_connected_clients_lock);
    boolset_foreach(&e->connected_clients_fds, disconnect_all_foreach, e);
    pthread_mutex_unlock(&e->_connected_clients_lock);

    // Empty the disconnection queue
    endpoint_clear_disconnection_queue(e);
}



int ept_own(endpoint2* e, int socket_fd) {
    return boolset_get(&e->connected_clients_fds, socket_fd);
}