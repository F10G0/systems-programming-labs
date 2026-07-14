#include <atomic>
#include <iostream>
#include <mutex>
#include <poll.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "utils.h"

// Shared counter and synchronized output.
std::atomic<int64_t> number{0};
std::mutex print_mutex;

// State owned by one server worker thread.
struct Worker {
    std::thread thread;
    std::mutex mtx;
    std::vector<int> pending_clients;
    std::vector<pollfd> active_clients;
};

// Process one message from a client.
// Returns true when the connection should be closed.
bool handle_client(int clientfd) {
    int32_t operation_type;
    int64_t argument;
    if (recv_msg(clientfd, &operation_type, &argument)) return true;
    
    switch(operation_type) {
        case OPERATION_ADD:
            number.fetch_add(argument);
            return false;
        case OPERATION_SUB:
            number.fetch_sub(argument);
            return false;
        case OPERATION_TERMINATION: {
            // Return the current counter value before closing the connection.
            int64_t ctr = number.load();
            {
                std::lock_guard<std::mutex> lock(print_mutex);
                std::cout << ctr << std::endl;
            }
            send_msg(clientfd, OPERATION_COUNTER, ctr);
            return true;
        }
        default:
            return true;
    }
}

// Poll and process all clients assigned to one worker.
void server_kernel(Worker *worker) {
    while (true) {
        // Move newly assigned clients into the worker's active set.
        {
            std::lock_guard<std::mutex> lock(worker->mtx);
            for (int client : worker->pending_clients) {
                worker->active_clients.push_back({client, POLLIN, 0});
            }
            worker->pending_clients.clear();
        }
        
        auto &clients = worker->active_clients;
        if (poll(clients.data(), clients.size(), 100) <= 0) continue;

        // Traverse backwards so finished clients can be removed safely.
        for (int i = clients.size() - 1; i >= 0; i--) {
            if (!(clients[i].revents & POLLIN)) continue;

            if (handle_client(clients[i].fd)) {
                close(clients[i].fd);
                clients[i] = clients.back();
                clients.pop_back();
            }
        }
    }
}

int main(int args, char *argv[]) {
    if (args < 3) {
        std::cerr << "usage: ./server <numThreads> <port>\n";
        exit(1);
    }

    int numThreads = std::atoi(argv[1]);
    int port = std::atoi(argv[2]);

    // Create the listening socket.
    int listenfd = listening_socket(port);
    if (listenfd < 0) return 1;

    // Start the fixed worker thread pool.
    std::vector<Worker> workers(numThreads);
    for (int i = 0; i < numThreads; i++) {
        workers[i].thread = std::thread(server_kernel, &workers[i]);
    }

    // Accept connections and distribute them round-robin.
    int connections = 0;
    while (true) {
        int acceptfd = accept_connection(listenfd);
        if (acceptfd < 0) continue;

        int tid = connections++ % numThreads;
        {
            std::lock_guard<std::mutex> lock(workers[tid].mtx);
            workers[tid].pending_clients.push_back(acceptfd);
        }
    }

    close(listenfd);
    return 0;
}
