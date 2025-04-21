#include "worker_thread.h"
#include "queue.h"
#include "common.h"
#include "task.h" 
#include <stdio.h>
#include <stdlib.h>


void* do_work(void* arg) {
    worker_thread_params_t* params = (worker_thread_params_t*)arg;
    queue_t* que = params->que;
    int thread_id = params->thread_id;

    printf("thread-%d starting\n", thread_id);
    free(params); // Free the parameters after the thread has started

    while (1) {
        request_t* req = (request_t*)queue_dequeue(que);

        if (req == NULL) {
            // Queue is closed and empty, time to exit
            printf("thread-%d exiting\n", thread_id);
            pthread_exit(NULL);
        }

        // Execute the task
        if (req->work_fn) {
            req->work_fn(req->arg);
        }
        free(req); // Free the request structure itself
    }

    // This point should not be reached in this implementation
    return NULL;
}

// Helper function to create a worker thread (as called from thread_pool_init)
pthread_t create_worker_thread(worker_thread_params_t* params) {
    pthread_t thread;
    int result = pthread_create(&thread, NULL, do_work, (void*)params);
    if (result != 0) {
        perror("pthread_create failed");
        free(params);
        return 0; // Indicate failure
    }
    return thread;
}