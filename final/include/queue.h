// queue.h
#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>

// Forward declaration of the queue structure
typedef struct queue_s queue_t;

// Opaque structure definition (implementation details are hidden)
struct queue_s {
    pthread_mutex_t *mutex;
    pthread_cond_t *not_empty;
    struct node_s *head;
    struct node_s *tail;
    int count;
    int closed; // Flag to indicate if the queue is closed
};

// Function to create and initialize a new queue
queue_t* queue_create(pthread_mutex_t* mutex, pthread_cond_t* not_empty);

// Function to enqueue (add) data to the back of the queue
int queue_enqueue(queue_t* q, void* data);

// Function to dequeue (remove) data from the front of the queue
void* queue_dequeue(queue_t* q);

// Function to close the queue (no more enqueues allowed)
void queue_close(queue_t* q);

// Function to destroy the queue and free its resources
void queue_destroy(queue_t* q);

int queue_is_empty(queue_t *q);

#endif
