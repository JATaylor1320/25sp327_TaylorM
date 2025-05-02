#define _GNU_SOURCE

#include "http_proxy.h"
#include "../include/common.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 4096 // size of buffer for recv/send of data

static void forward_request(int target_socket, char buffer[]);
static void forward_response(int target_socket, int client_socket);
static int connect_to_target_server(const char *hostname, int port);
static bool parse_http_request(const char *buffer, char *hostname, int *port);
static bool receive_client_request(int client_socket, char *buffer);

/**
 * @brief The entry point to handle a client requests on a pthread.
 * @param arg the client socket
 */
void handle_client_request_thread(void *arg) {
    int client_socket = *((int *)arg);
    free(arg);
    handle_client_request(client_socket);
}

/**
 * @brief The main logic for handling a client request
 * @param client_socket
 */
void handle_client_request(int client_socket) {
    char buffer[BUFFER_SIZE];

    // Receive the HTTP request from the client
    // buffer contains the request data upon success
    if (!receive_client_request(client_socket, buffer)) {
        return;
    }

    char hostname[256];
    int port;

    // Parse the HTTP request received from the client
    // hostname and port will be populated upon success
    if (!parse_http_request(buffer, hostname, &port)) {
        return;
    }

    // Connect to the target server - the server the client wants to
    // connect to.
    int target_socket = connect_to_target_server(hostname, port);
    if (target_socket < 0) {
        return;
    }

    // Forward the clients request to the target server
    forward_request(target_socket, buffer);

    // Receive response from target and forward to client
    forward_response(target_socket, client_socket);

    close(target_socket);
    close(client_socket);
    printf("Request completed successfully.\n");
}

/**
 * @brief Forwards the request to the target socket
 * @param target_socket
 * @param buffer
 */
static void forward_request(int target_socket, char buffer[]) {
    char method[10], url[1024], protocol[10], path[1024];
    memset(path, 0, sizeof(path));

    // Parse the request line: extract method, full URL, and protocol
    if (sscanf(buffer, "%s %s %s", method, url, protocol) != 3) {
        perror("Failed to parse request line");
        return;
    }

    // Extract the path from the full URL (skip http://host:port)
    if (strncmp(url, "http://", 7) == 0) {
        char *path_start = strchr(url + 7, '/');  // Find the first '/' after 'http://host'
        if (path_start) {
            strncpy(path, path_start, sizeof(path) - 1);  // Copy the path portion
        } else {
            strcpy(path, "/");  // Default path if none specified
        }
    } else {
        perror("Invalid HTTP request URL");
        return;
    }

    // Reconstruct the HTTP request line with the correct path
    char modified_request[BUFFER_SIZE * 2]; // Increased buffer size to be safe
    snprintf(modified_request, sizeof(modified_request), "%s %s %s\r\n", method, path, protocol);

    // Locate the rest of the HTTP headers (skip the first line)
    char *headers = strstr(buffer, "\r\n");
    if (headers) {
        strncat(modified_request, headers + 2, sizeof(modified_request) - strlen(modified_request) - 1);
    }
    strcat(modified_request, "\r\n"); // Ensure the request ends with a blank line

    printf("Forwarding request to target:\n%s\n", modified_request);

    // Send the modified request to the target server
    ssize_t bytes_sent = send(target_socket, modified_request, strlen(modified_request), MSG_NOSIGNAL);
    if (bytes_sent == -1) {
        perror("Send to target server failed");
    }
}

static bool receive_client_request(int client_socket, char *buffer) {
    memset(buffer, 0, BUFFER_SIZE);
    ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        perror("Failed to receive data from client");
        return false;
    }
    buffer[bytes_received] = '\0'; // Null-terminate the received data
    printf("Request received from client:\n%s\n", buffer);
    return true;
}

static bool parse_http_request(const char *buffer, char *hostname, int *port) {
    char method[10], url[1024], protocol[10];
    if (sscanf(buffer, "%s %s %s", method, url, protocol) != 3) {
        fprintf(stderr, "Failed to parse HTTP request line\n");
        return false;
    }

    *port = 80; // Default HTTP port

    if (strncmp(url, "http://", 7) == 0) {
        if (sscanf(url + 7, "%255[^:/]:%d", hostname, port) == 2) {
            printf("Hostname: %s, Port: %d\n", hostname, *port);
        } else if (sscanf(url + 7, "%255[^/]", hostname) == 1) {
            printf("Hostname: %s, Port: %d (default)\n", hostname, *port);
        } else {
            perror("Failed to parse hostname or port");
            return false;
        }
    } else if (strncmp(url, "https://", 8) == 0) {
        fprintf(stderr, "Only HTTP requests supported at this stage\n");
        return false;
    } else {
        fprintf(stderr, "Unexpected URL '%s'\n", url);
        fprintf(stderr, "Proxy requires absolute URL to target server.\n");
        return false;
    }

    return true;
}

/**
 * @brief Connects to the target server
 * @param hostname the target host
 * @param port the target port
 * @return the socket for the target server or -1 on failure to connect
 */
static int connect_to_target_server(const char *hostname, int port) {
    struct hostent *server;
    struct sockaddr_in server_addr;
    int sockfd;

    // Resolve the hostname to an IP address
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "ERROR: No such host as %s\n", hostname);
        return -1;
    }

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR: Opening socket");
        return -1;
    }

    // Configure the server address structure
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(port);

    // Connect to the target server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR: Connecting to target server");
        close(sockfd);
        return -1;
    }

    printf("Connected to target server: %s:%d\n", hostname, port);
    return sockfd;
}

/**
 * @brief Receives the response from the target socket and sends it to the client socket.
 * @param target_socket
 * @param client_socket
 */
static void forward_response(int target_socket, int client_socket) {
    ssize_t bytes_received;
    char buffer[BUFFER_SIZE * 2]; // Increased buffer size for response

    printf("Forwarding response from target to client...\n");

    while ((bytes_received = recv(target_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0'; // Null-terminate the received data
        printf("Received %zd bytes from target.\n", bytes_received);

        ssize_t bytes_sent = send(client_socket, buffer, bytes_received, MSG_NOSIGNAL);
        if (bytes_sent < 0) {
            perror("ERROR: Sending response to client");
            break;
        }
        printf("Sent %zd bytes to client.\n", bytes_sent);
    }

    if (bytes_received < 0) {
        perror("ERROR: Receiving response from target");
    } else if (bytes_received == 0) {
        printf("Target server closed the connection.\n");
    }

    printf("Finished forwarding response.\n");
}