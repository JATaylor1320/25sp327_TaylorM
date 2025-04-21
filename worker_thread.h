#ifndef WORKER_THREAD_H
#define WORKER_THREAD_H

#include <pthread.h>
#include "queue.h" // Include for queue_t if needed here

// Structure to hold parameters passed to the worker thread
typedef struct worker_thread_params_s{
    queue_t* que;
    int thread_id;
} worker_thread_params_t;

// Function executed by each worker thread
void* do_work(void* arg);

// Helper function to create a worker thread
static pthread_t create_worker_thread(worker_thread_params_t* params);

#endif