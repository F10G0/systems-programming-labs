#include "utils.h"
#include "message.pb.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

// Export the utility functions with C linkage.
extern "C" {

int listening_socket(int port) {
    // Create an IPv4 TCP socket.
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    // Allow the port to be reused shortly after the server exits.
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind the socket to localhost and the requested port.
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(fd, (sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }

    // Mark the socket as a listening socket.
    if (listen(fd, SOMAXCONN) < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

int connect_socket(const char *hostname, const int port) {
    // Resolve the hostname to an IPv4 address.
    hostent *host = gethostbyname(hostname);
    if (!host) return -1;

    // Create an IPv4 TCP socket.
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    // Construct the server address.
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr, host->h_addr, host->h_length);

    // Establish the connection.
    if (connect(fd, (sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

int accept_connection(int sockfd) {
    // Accept one pending client connection.
    return accept(sockfd, nullptr, nullptr);
}

int recv_msg(int sockfd, int32_t *operation_type, int64_t *argument) {
    // Read the length-prefixed protobuf payload size.
    uint32_t size;
    if (recv(sockfd, &size, sizeof(size), 0) <= 0) return 1;
    size = ntohl(size);

    // Read the complete serialized payload.
    std::string payload;
    payload.resize(size);
    char *buf = payload.data();
    for (size_t offset = 0; offset < size;) {
        ssize_t n = recv(sockfd, buf + offset, size - offset, 0);
        if (n <= 0) return 1;
        offset += n;
    }

    // Deserialize the protobuf message.
    sockets::message msg;
    if (!msg.ParseFromString(payload)) return 1;
    *operation_type = msg.type();
    *argument = msg.has_argument() ? msg.argument() : 0;

    return 0;
}

int send_msg(int sockfd, int32_t operation_type, int64_t argument) {
    // Construct and serialize the protobuf message.
    sockets::message msg;
    msg.set_type((sockets::message::OperationType)operation_type);
    msg.set_argument(argument);

    std::string payload;
    msg.SerializeToString(&payload);

    // Send the payload size in network byte order.
    uint32_t size = htonl((uint32_t)payload.size());
    if (send(sockfd, &size, sizeof(size), 0) <= 0) return 1;

    // Send the complete serialized payload.
    const char *buf = payload.data();
    size_t len = payload.size();
    for (size_t offset = 0; offset < len;) {
        ssize_t n = send(sockfd, buf + offset, len - offset, 0);
        if (n <= 0) return 1;
        offset += n;
    }

    return 0;
}

}
