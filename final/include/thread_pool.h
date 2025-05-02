#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "queue.h" 
#include <pthread.h>
#include <stdlib.h> // For malloc and free

#define MAX_WORKER_THREADS 20

// Define the structure for the thread pool
typedef struct thread_pool {
    pthread_t *threads;     // Array of thread IDs
    int num_threads;        // Number of threads in the pool
    int next_thread_id;
    queue_t *task_queue;     // Queue for tasks
    pthread_mutex_t queue_mutex; // Mutex for queue access
    pthread_cond_t queue_cond;   // Condition variable for queue
    int is_shutdown;        // Flag to indicate if the pool is shutting down
} thread_pool_t;

// Structure to hold arguments for worker threads (if needed)
typedef struct worker_thread_params {
    thread_pool_t *pool;
    // You can add other parameters if your worker thread function needs them
} worker_thread_params_t;

// Function declarations
thread_pool_t *thread_pool_create(int num_threads, queue_t *task_queue);
void thread_pool_destroy(thread_pool_t *pool);
int thread_pool_submit(thread_pool_t *pool, void (*process)(void *), void *arg);

// The function that worker threads will execute
void *worker_thread(void *arg);

#endif // THREAD_POOL_H