#ifndef WORKER_THREAD_H
#define WORKER_THREAD_H

#include <pthread.h>
#include "queue.h"

typedef struct worker_thread_s {
    pthread_t thread;
    int id;
    // ... other members ...
} worker_thread_t;

void* do_work(void* arg);

// Function declarations related to worker threads
static pthread_t create_worker_thread(worker_thread_params_t* params);

#endif // WORKER_THREAD_H