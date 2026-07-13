#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <unistd.h>

#include "utils.h"

// Prevent concurrent client threads from interleaving their output.
static std::mutex print_mutex;

// Connect to the server, send all operations, and receive the final counter.
void client_kernel(std::string hostname, int port, int numMessages, int add, int sub) {
    int sockfd = connect_socket(hostname.c_str(), port);
    if (sockfd < 0) return;

    // Send alternating ADD and SUB messages, starting with ADD.
    for (int i = 1; i <= numMessages; i++) {
        if (i % 2) {
            if (send_msg(sockfd, OPERATION_ADD, add)) goto cleanup;
        } else {
            if (send_msg(sockfd, OPERATION_SUB, sub)) goto cleanup;
        }
    }

    // Notify the server that this client has finished.
    if (send_msg(sockfd, OPERATION_TERMINATION, 0)) goto cleanup;

    // Receive and print the counter returned by the server.
    int32_t operation_type;
    int64_t argument;
    if (recv_msg(sockfd, &operation_type, &argument) == 0) {
        if (operation_type != OPERATION_COUNTER) goto cleanup;
        
        std::lock_guard<std::mutex> lock(print_mutex);
        std::cout << argument << std::endl;
    }

cleanup:
    // Close the connection on both success and failure.
    close(sockfd);
}

int main(int args, char *argv[]) {
    // Parse the client configuration from the command line.
    if (args < 7) {
        std::cerr << "usage: ./client <num_threads> <hostname> <port> <num_messages> <add> <sub>\n";
        exit(1);
    }

    int numClients = std::atoi(argv[1]);
    std::string hostname = argv[2];
    int port = std::atoi(argv[3]);
    int numMessages = std::atoi(argv[4]);
    int add = std::atoi(argv[5]);
    int sub = std::atoi(argv[6]);

    std::vector<std::thread> threads;

    // Start independent client connections.
    for (int i = 0; i < numClients; i++) {
        threads.emplace_back(client_kernel, hostname, port, numMessages, add, sub);
    }

    // Wait until all clients have completed.
    for (auto &t: threads) {
        t.join();
    }

    return 0;
}
