#ifndef COMMON_H
#define COMMON_H

#define BUFFER_SIZE 4096 // Make sure this matches the definition in http_proxy.h

// Definition of the request structure
struct request_s {
  int client_fd;
    
};

typedef struct request_s request_t;

#endif // COMMON_H