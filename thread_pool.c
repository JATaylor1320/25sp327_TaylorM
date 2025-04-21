#include "thread_pool.h"
#include "common.h"
#include "worker_thread.h"
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

// Helper function to create a worker thread
static pthread_t create_worker_thread(worker_thread_params_t* params) {
    pthread_t thread;
    int result = pthread_create(&thread, NULL, do_work, (void*)params);
    if (result != 0) {
        perror("pthread_create failed");
        free(params);
        return 0; // Indicate failure
    }
    return thread;
}

struct thread_pool* thread_pool_init(queue_t* que, int num_threads) {
    struct thread_pool* pool = malloc(sizeof(struct thread_pool));
    if (!pool) {
        perror("thread_pool_init: malloc failed");
        return NULL;
    }
    pool->next_thread_id = 1;
    pool->num_threads = num_threads;
    pool->que = que;

    if (num_threads > MAX_WORKER_THREADS) {
        fprintf(stderr, "Warning: Requested %d threads exceeds maximum of %d. Using %d threads.\n",
                num_threads, MAX_WORKER_THREADS, MAX_WORKER_THREADS);
        num_threads = MAX_WORKER_THREADS;
        pool->num_threads = MAX_WORKER_THREADS;
    }

    for (int i = 0; i < pool->num_threads; i++) {
        worker_thread_params_t* params = malloc(sizeof(worker_thread_params_t));
        if (!params) {
            perror("thread_pool_init: malloc failed for thread params");
            // Clean up previously allocated resources
            for (int j = 0; j < i; j++) {
                if (pool->threads[j]) {
                    free(pool->threads[j]);
                }
            }
            free(pool);
            return NULL;
        }
        params->que = que;
        params->thread_id = pool->next_thread_id;
        pool->threads[i] = malloc(sizeof(struct worker_thread)); // Allocate memory for worker_thread struct
        if (!pool->threads[i]) {
            perror("thread_pool_init: malloc failed for worker_thread");
            free(params);
            // Clean up previously allocated resources
            for (int j = 0; j < i; j++) {
                if (pool->threads[j]) {
                    free(pool->threads[j]);
                }
            }
            free(pool);
            return NULL;
        }
        pool->threads[i]->thread_id = pool->next_thread_id;
        pool->threads[i]->thread = create_worker_thread(params);
        if (pool->threads[i]->thread == 0) {
            // Thread creation failed, clean up
            free(pool->threads[i]);
            free(params);
            // Clean up previously allocated resources
            for (int j = 0; j < i; j++) {
                if (pool->threads[j]) {
                    free(pool->threads[j]);
                }
            }
            free(pool);
            return NULL;
        }
        pool->next_thread_id++;
    }

    return pool;
}

void thread_pool_destroy(struct thread_pool* pool) {
    if (pool) {
        for (int i = 0; i < pool->num_threads; i++) {
            if (pool->threads[i]) {
                pthread_join(pool->threads[i]->thread, NULL);
                free(pool->threads[i]); // Free the allocated worker_thread struct
                pool->threads[i] = NULL; // Good practice to set to NULL after freeing
            }
        }
        free(pool); // Free the thread_pool struct itself
    }
}