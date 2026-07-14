#pragma once

#include <stdint.h>

// Client -> Server
const int32_t OPERATION_ADD = 1;
const int32_t OPERATION_SUB = 2;
const int32_t OPERATION_TERMINATION = 3;

// Server -> Client
const int32_t OPERATION_COUNTER = 4;

#ifdef __cplusplus
extern "C" {
#endif

// Create a TCP listening socket.
// Returns the socket fd, or -1 on failure.
int listening_socket(int port);

// Connect to a TCP server.
// Returns the socket fd, or -1 on failure.
int connect_socket(const char *hostname, const int port);

// Accept a new connection.
// Returns the client socket fd, or -1 on failure.
int accept_connection(int sockfd);

// Receive one complete message.
// Returns 0 on success, 1 on failure.
int recv_msg(int sockfd, int32_t *operation_type, int64_t *argument);

// Send one complete message.
// Returns 0 on success, 1 on failure.
int send_msg(int sockfd, int32_t operation_type, int64_t argument);

#ifdef __cplusplus
}
#endif
