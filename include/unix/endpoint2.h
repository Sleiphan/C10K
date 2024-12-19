#ifndef CUSTOM_UNIX_SOCKET_ENDPOINT
#define CUSTOM_UNIX_SOCKET_ENDPOINT

#include <stdbool.h>
#include <sys/epoll.h>

#include "unix/workers.h"
#include "unix/boolset.h"
#include "unix/stack.h"



/// @brief An endpoint is a point of entry for clients to connect.
/// It keeps a live record of all clients currently connected through it,
/// and is specced to handle billions of concurrently connected clients.
typedef struct endpoint2 {
    int _server_fd; // File descriptor id for the server socket
    int _epoll_instance; // Epoll-instance for all sockets, including server socket
    uint32_t _epoll_event_whitelist; // Specifies which epoll events to listen for

    bool _connections_waiting;
    
    /// @brief This is the record of currently connected clients. The use of 'boolset_t' gives a complexity of O(1) for
    /// establishing and closing client connections, meaning that performance does not drop with a rising amount of connected clients.
    boolset_t connected_clients_fds;

    pthread_mutex_t _connected_clients_lock;
    
    stack _closing_queue;
} endpoint2;



#ifndef EPOLLRDCON
#define EPOLLRDCON 0x1000
#endif



void endpoint2_create(endpoint2* s, int port_number, bool IPv6);

/// @brief Completely clean up after an endpoint. This includes:
///  1. Immediately block the listening socket and forcefully disconnect all clients connected through this endpoint.
///  2. Closes all file descriptors and deallocate all dynamically allocated memory associated with the specified endpoint.
/// @param e The endpoint to close.
void endpoint2_close(endpoint2* e);

/// @brief
/// Please note that EPOLLRDCON is an addition to the Epoll event codes, and that it is exclusive to all
/// @param e 
/// @param events 
/// @param max_events On input: contains the size of the 'events' array. On output: contains the count of returned events.
/// @param timeout 
/// @return 0 if the job succeeded, or -1 if an error occurred. In the event of an error, errno will be properly set.
int endpoint2_step(endpoint2* e, struct epoll_event events[], int* max_events, const int timeout);

/// @brief Disconnect a specific client from this endpoint.
/// @param e The endpoint the client connected through.
/// @param socket_fd The file descriptor of the client's socket.
/// @return 
int endpoint2_disconnect_client(endpoint2* e, int socket_fd);

/// @brief Immediately disconnects all clients that originated from this endpoint.
/// Their sockets will be closed, and their file descriptors released.
/// @param e The endpoint to disconnect clients from.
/// @return The total number of clients disconnected.
int endpoint2_disconnect_all_clients(endpoint2* e);




/// @brief Checks whether the given socket established
/// a connection through the given endpoint. 
/// @param e The endpoint
/// @param socket_fd The file descriptor id of the socket in question.
/// @return 1 if the socket connected through this endpoint. Otherwise, returns 0.
int ept_own(endpoint2* e, int socket_fd);



#endif // CUSTOM_UNIX_SOCKET_ENDPOINT