#define _GNU_SOURCE

#include "../include/common.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

#include "http_proxy.h"
#include "../include/queue.h"
#include "../include/thread_pool.h"

#define LISTEN_PORT 8080  // Proxy listens on this port

// Global pointer to the thread pool for cleanup
thread_pool_t *tp = NULL;
queue_t *q = NULL;
int server_fd = -1;

// Signal handler for SIGINT (Ctrl+C)
void sigint_handler(int sig) {
    fprintf(stderr, "\nShutting down proxy server...\n");
    if (tp != NULL) {
        thread_pool_destroy(tp);
    }
    if (q != NULL) {
        queue_destroy(q);
    }
    if (server_fd != -1) {
        close(server_fd);
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
    // Accept command-line arguments for the number of threads
    int num_threads = 2;    // provide a default number of threads
    if (argc == 2) {
        num_threads = atoi(argv[1]);
    }

    // TODO Initialize a queue and thread pool to handle requests
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_not_empty;

    if (pthread_mutex_init(&queue_mutex, NULL) != 0) {
        perror("pthread_mutex_init failed");
        exit(EXIT_FAILURE);
    }
    if (pthread_cond_init(&queue_not_empty, NULL) != 0) {
        perror("pthread_cond_init failed");
        pthread_mutex_destroy(&queue_mutex);
        exit(EXIT_FAILURE);
    }

    q = queue_create(&queue_mutex, &queue_not_empty);
    if (!q) {
        perror("Failed to create queue");
        exit(EXIT_FAILURE);
    }

    tp = thread_pool_create(num_threads, q);
    if (!tp) {
        perror("Failed to create thread pool");
        queue_destroy(q);
        exit(EXIT_FAILURE);
    }

    // TODO Create socket for the proxy server to listen on the LISTEN_PORT
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        thread_pool_destroy(tp);
        queue_destroy(q);
        exit(EXIT_FAILURE);
    }

    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt SO_REUSEADDR failed");
        close(server_fd);
        thread_pool_destroy(tp);
        queue_destroy(q);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(LISTEN_PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        thread_pool_destroy(tp);
        queue_destroy(q);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("listen failed");
        close(server_fd);
        thread_pool_destroy(tp);
        queue_destroy(q);
        exit(EXIT_FAILURE);
    }

    // TODO Add a signal handler to cleaning shutdown the server
    // when a SIGINT (ctrl-c) is received.
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction failed");
        close(server_fd);
        thread_pool_destroy(tp);
        queue_destroy(q);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Proxy server listening on port %d with %d threads...\n", LISTEN_PORT, num_threads);

    // TODO Accept incoming client connections. Add a request_t to the queue to
    // handle the client's HTTP request. The client socket is the argument to be
    // passed to the handle_client_request_thread function.
    // Continue to accept client connections until SIGINT is received.
    while (true) {
        struct sockaddr_in client_address;
        socklen_t client_addrlen = sizeof(client_address);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_addrlen);
        if (client_fd < 0) {
            // Check if the error was due to a signal interrupt (like Ctrl+C)
            if (errno == EINTR) {
                break; // Exit the loop and proceed to cleanup
            } else {
                perror("accept failed");
                continue; // Continue accepting other connections if possible
            }
        }

        request_t *req = malloc(sizeof(request_t));
        if (!req) {
            perror("Failed to allocate request_t");
            close(client_fd);
            continue;
        }
        req->client_fd = client_fd;

        if (queue_enqueue(q, req) != 0) {
            perror("Failed to enqueue request");
            free(req);
            close(client_fd);
        }
    }

    // TODO perform any clean up before server shutdown
    fprintf(stdout, "Cleaning up resources...\n");
    if (tp != NULL) {
        thread_pool_destroy(tp);
    }
    if (q != NULL) {
        queue_destroy(q);
    }
    if (server_fd != -1) {
        close(server_fd);
    }
    pthread_mutex_destroy(&queue_mutex);
    pthread_cond_destroy(&queue_not_empty);

    return EXIT_SUCCESS;
}