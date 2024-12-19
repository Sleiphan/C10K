#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/eventfd.h>
#include <sys/socket.h>

#include "unix/endpoint.h"

// ### HELPER FUNCTIONS ###

// Empty the disconnection queue of an endpoint
void endpoint_clear_disconnection_queue(endpoint *e)
{
    pthread_mutex_lock(&e->_disconnect_queue_lock);
    stack_destroy(&e->_disconnect_queue);
    pthread_mutex_unlock(&e->_disconnect_queue_lock);
}

void endpoint_register_client(endpoint *e, int socket_fd)
{
    pthread_mutex_lock(&e->_connected_clients_lock);
    boolset_set(&e->connected_clients_fds, socket_fd, 1);
    pthread_mutex_unlock(&e->_connected_clients_lock);
}

void endpoint_deregister_client(endpoint *e, int socket_fd)
{
    pthread_mutex_lock(&e->_connected_clients_lock);
    boolset_set(&e->connected_clients_fds, socket_fd, 0);
    pthread_mutex_unlock(&e->_connected_clients_lock);
}

void endpoint_enqueue_client_for_disconnection(endpoint *e, int socket_fd)
{
    pthread_mutex_lock(&e->_disconnect_queue_lock);
    stack_push(&e->_disconnect_queue, (void *)(intptr_t)socket_fd);
    pthread_mutex_unlock(&e->_disconnect_queue_lock);
}

// ### MORE HELPER FUNCTIONS ###

struct epoll_event create_epoll_event(int fd, uint32_t events)
{
    struct epoll_event event;
    event.events = events;
    event.data.fd = fd;

    return event;
}

int create_server_socket(int PORT_NUMBER, bool IPv6)
{
    const int type = SOCK_STREAM;
    const int domain = IPv6 ? AF_INET6 : AF_INET;

    int srv_fd = socket(domain, type, IPPROTO_TCP);

    // Make the socket non-blocking
    fcntl(srv_fd, F_SETFL, fcntl(srv_fd, F_GETFL, 0) | O_NONBLOCK);

    // The address to bind the socket to
    struct sockaddr_in server_address;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_family = domain;
    server_address.sin_port = htons(PORT_NUMBER);

    // Bind the socket
    bind(srv_fd, (struct sockaddr *)&server_address, sizeof(server_address));

    // Make the socket a server-socket
    listen(srv_fd, 512);

    return srv_fd;
}

// ### DECLARED FUNCTIONS ###

void endpoint_create(endpoint *e, int port_number, bool IPv6)
{
    pthread_mutex_init(&e->_connected_clients_lock, NULL);
    pthread_mutex_init(&e->_disconnect_queue_lock, NULL);

    e->connected_clients_fds = boolset_create();
    e->_disconnect_queue = stack_create();

    e->_srv_fd = create_server_socket(port_number, IPv6);

    e->_ep_listener = epoll_create1(0);
    e->_ep_selector = epoll_create1(0);
    e->_shutdown_fd = eventfd(0, 0);

    struct epoll_event srv_events = create_epoll_event(e->_srv_fd, EPOLLIN | EPOLLHUP);
    struct epoll_event sht_events = create_epoll_event(e->_shutdown_fd, EPOLLIN);

    epoll_ctl(e->_ep_listener, EPOLL_CTL_ADD, e->_srv_fd, &srv_events);
    epoll_ctl(e->_ep_listener, EPOLL_CTL_ADD, e->_shutdown_fd, &sht_events);
    epoll_ctl(e->_ep_selector, EPOLL_CTL_ADD, e->_shutdown_fd, &sht_events);
}

void endpoint_close(endpoint *e)
{
    close(e->_srv_fd);                  // Shutdown port for incoming connections.
    endpoint_disconnect_all_clients(e); // Forcefully disconnect all connected clients.

    // Clean the epoll instances
    epoll_ctl(e->_ep_listener, EPOLL_CTL_DEL, e->_srv_fd, NULL);
    epoll_ctl(e->_ep_listener, EPOLL_CTL_DEL, e->_shutdown_fd, NULL);
    epoll_ctl(e->_ep_selector, EPOLL_CTL_DEL, e->_shutdown_fd, NULL);

    // Close all file descriptors
    close(e->_ep_listener);
    close(e->_ep_selector);
    close(e->_shutdown_fd);

    // Deallocate the boolset that keeps track of connected clients.
    pthread_mutex_lock(&e->_connected_clients_lock);
    boolset_destroy(&e->connected_clients_fds);
    pthread_mutex_unlock(&e->_connected_clients_lock);

    // Empty the disconnection queue of an endpoint
    endpoint_clear_disconnection_queue(e);

    pthread_mutex_destroy(&e->_connected_clients_lock);
    pthread_mutex_destroy(&e->_disconnect_queue_lock);
}

int endpoint_accept(endpoint *e, int *new_fds, const int new_fds_size, int timeout, const uint32_t events_flags)
{
    // Receives the event struct from epoll_wait
    struct epoll_event ep_event;

    // Wait for client events
    epoll_wait(e->_ep_listener, &ep_event, 1, timeout);

    // Exit if the server-socket has been closed.
    if (ep_event.events & EPOLLHUP)
        return -1;

    int new_client_count = 0;       // Keeps count of how many clients we have accepted during this call to 'endpoint_accept'
    int new_client_fd = -1;         // Receives the file descriptor of the client's socket
    struct sockaddr client_address; // Receives the client address
    socklen_t client_address_len;   // Receives the length of the client address

    for (; new_client_count < new_fds_size && (new_client_fd = accept(e->_srv_fd, &client_address, &client_address_len)) != -1; ++new_client_count)
    {
        // Register the client in the boolset
        endpoint_register_client(e, new_client_fd);

        // Create event-object, making sure we allways listen to EPOLLRDHUP
        struct epoll_event client_event = create_epoll_event(new_client_fd, events_flags | EPOLLRDHUP);

        // Tell the caller about the new connection
        new_fds[new_client_count] = new_client_fd;

        // Add client to the epoll-instance for client events
        epoll_ctl(e->_ep_selector, EPOLL_CTL_ADD, new_client_fd, &client_event);
    }

    return new_client_count;
}

int endpoint_wait_event(endpoint *e, int *socket_fds, int *event_flags, const int max_events, int timeout)
{
    // The maximum amount of clients events to extract during this
    static const int EVENTS_EXTRACT_SIZE = (1 << 9) / sizeof(struct epoll_event);

    // The maximum amount of events to extract from the epoll instance
    const int event_limit = max_events < EVENTS_EXTRACT_SIZE ? max_events : EVENTS_EXTRACT_SIZE;

    // The buffer to store events
    struct epoll_event ep_events[EVENTS_EXTRACT_SIZE];

    // Disconnect and close any client sockets queued for disconnection.
    // endpoint_close_hungups(e);

    const int event_count = epoll_wait(e->_ep_selector, ep_events, event_limit, timeout);

    for (int i = 0; i < event_count; ++i)
    {
        socket_fds[i] = ep_events[i].data.fd;
        event_flags[i] = ep_events[i].events;

        // Disconnect the client if they close their own socket
        if (ep_events[i].events & EPOLLRDHUP)
            endpoint_enqueue_client_for_disconnection(e, socket_fds[i]);
    }

    return event_count;
}

inline int endpoint_close_hungups(endpoint *e)
{
    int count = 0;

    pthread_mutex_lock(&e->_disconnect_queue_lock);
    for (; !stack_is_empty(e->_disconnect_queue); ++count)
        endpoint_disconnect_client(e, (intptr_t)stack_pop(&e->_disconnect_queue));
    pthread_mutex_unlock(&e->_disconnect_queue_lock);

    return count;
}

inline int endpoint_disconnect_client(endpoint *e, int socket_fd)
{
    // Remove the client from the list of connected clients
    endpoint_deregister_client(e, socket_fd);

    // Remove the client from the selector epoll instance
    epoll_ctl(e->_ep_selector, EPOLL_CTL_DEL, socket_fd, NULL);

    // Shut down both channels of communication
    shutdown(socket_fd, SHUT_RDWR);

    // Close the client's socket
    close(socket_fd);

    // Return success
    return 0;
}

void disconnect_all_foreach(BOOLSET_TYPE socket_fd, void *args)
{
    endpoint *e = (endpoint *)args;

    boolset_set(&e->connected_clients_fds, socket_fd, 0);

    // Remove the client from the selector epoll instance
    epoll_ctl(e->_ep_selector, EPOLL_CTL_DEL, socket_fd, NULL);

    // Shut down both channels of communication
    shutdown(socket_fd, SHUT_RDWR);

    // Close the client's socket
    close(socket_fd);
}

inline int endpoint_disconnect_all_clients(endpoint *e)
{
    pthread_mutex_lock(&e->_connected_clients_lock);
    boolset_foreach(&e->connected_clients_fds, disconnect_all_foreach, e);
    pthread_mutex_unlock(&e->_connected_clients_lock);

    // Empty the disconnection queue
    endpoint_clear_disconnection_queue(e);
}