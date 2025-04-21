// main.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "queue.h"
#include "thread_pool.h"
#include "task.h"
#include "common.h"
#include <string.h>

// Function to create a request
request_t* create_request(void (*work_fn)(void*), void* arg) {
    request_t* request = malloc(sizeof(request_t));
    if (!request) {
        perror("create_request: malloc failed");
        return NULL;
    }
    request->work_fn = work_fn;
    request->arg = arg;
    return request;
}

int main(int argc, char *argv[]) {
    int num_threads = 4;
    queue_t* task_queue;
    pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;

    task_queue = queue_create(&queue_mutex, &queue_not_empty);
    if (!task_queue) {
        fprintf(stderr, "Failed to create task queue.\n");
        return EXIT_FAILURE;
    }

    struct thread_pool* pool = thread_pool_init(task_queue, num_threads);
    if (!pool) {
        fprintf(stderr, "Failed to initialize thread pool.\n");
        queue_destroy(task_queue);
        return EXIT_FAILURE;
    }

    char* log_msg1 = strdup("First log message from main.");
    request_t* log_req1 = create_request(write_log, log_msg1);
    queue_enqueue(task_queue, log_req1);

    int* nums1 = malloc(sizeof(int) * 2);
    nums1[0] = 25;
    nums1[1] = 17;
    request_t* sum_req1 = create_request(compute_sum, nums1);
    queue_enqueue(task_queue, sum_req1);

    int* delay_time1 = malloc(sizeof(int));
    *delay_time1 = 1;
    request_t* delay_req1 = create_request(delay_task, delay_time1);
    queue_enqueue(task_queue, delay_req1);

    for (int i = 0; i < 3; i++) {
        char* log_msg = malloc(30);
        sprintf(log_msg, "Loop message %d from main.", i);
        request_t* log_req = create_request(write_log, log_msg);
        queue_enqueue(task_queue, log_req);

        int* nums = malloc(sizeof(int) * 2);
        nums[0] = i * 5;
        nums[1] = i * 5 + 1;
        request_t* sum_req = create_request(compute_sum, nums);
        queue_enqueue(task_queue, sum_req);

        int* delay_time = malloc(sizeof(int));
        *delay_time = 0; // Short delay
        request_t* delay_req = create_request(delay_task, delay_time);
        queue_enqueue(task_queue, delay_req);
    }

    queue_close(task_queue);
    printf("Task queue closed from main.\n");

    thread_pool_destroy(pool);
    printf("Thread pool destroyed from main.\n");

    return EXIT_SUCCESS;
}