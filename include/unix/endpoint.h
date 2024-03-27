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
typedef struct endpoint {
    int _srv_fd; // File descriptor id for the server socket
    int _ep_listener; // Epoll-instance for the server socket
    int _ep_selector; // Epoll-instance for all client sockets
    int _shutdown_fd; // As of now, unsused
    
    /// @brief This is the record of currently connected clients. The use of 'boolset_t' gives a complexity of O(1) for
    /// establishing and closing client connections, meaning that performance does not drop with a rising amount of connected clients.
    boolset_t connected_clients_fds;

    pthread_mutex_t _connected_clients_lock;
    pthread_mutex_t _disconnect_queue_lock;

    stack _disconnect_queue;
} endpoint;

void endpoint_create(endpoint* s, int port_number, bool IPv6);

/// @brief Completely clean up after an endpoint. This includes:
///  1. Immediately block the listening socket and forcefully disconnect all clients connected through this endpoint.
///  2. Closes all file descriptors and deallocate all dynamically allocated memory associated with the specified endpoint.
/// @param e The endpoint to close.
void endpoint_close(endpoint* e);

/// @brief Blocks until a new client connects
/// @param e 
/// @param timeout 
/// @param events_flags 
/// @return The number of new clients accepted, or -1 if the endpoint has closed.
int endpoint_accept(endpoint* e, int* new_fds, int new_fds_size, int timeout, uint32_t events_flags);

/// @brief Blocks until a client event happens.
/// @param e 
/// @param timeout The maximum amount of time to wait for events, in milliseconds.
/// @param socket_fds The location to store the ready file descriptors.
/// @param event_flags The location to store the event flag for the ready file descriptors.
/// @param max_events The maximum amount of events to write to 'socket_fd' and 'event_flags'.
/// @return The number of events written to 'socket_fd' and 'event_flags', or -1 if the endpoint has closed.
int endpoint_wait_event(endpoint* e, int* socket_fds, int* event_flags, const int max_events, int timeout);

/// @brief Disconnects any clients that have closed the socket on their end. Should be called regularly to keep the
/// client pool clean. 
/// A client is added to a disconnection-queue by the 'endpoint_wait_event'-function, when the epoll-instance returns
/// the EPOLLRDHUP-event for the specific client. This function ('endpoint_close_hungups') closes all sockets
/// in this queue.
/// @param e 
/// @return The number of hung up client sockets closed.
int endpoint_close_hungups(endpoint* e);

/// @brief Disconnect a specific client from this endpoint.
/// @param e The endpoint the client connected through.
/// @param socket_fd The file descriptor of the client's socket.
/// @return 
int endpoint_disconnect_client(endpoint* e, int socket_fd);

/// @brief Immediately disconnects all clients that originated from this endpoint.
/// Their sockets will be closed, and their file descriptors released.
/// @param e The endpoint to disconnect clients from.
/// @return The total number of clients disconnected.
int endpoint_disconnect_all_clients(endpoint* e);



#endif // CUSTOM_UNIX_SOCKET_ENDPOINT