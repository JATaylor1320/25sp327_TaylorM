// queue.c
#include "queue.h"
#include "queue_internal.h" // Assuming node_t is defined here
#include <stdlib.h>
#include <stdio.h>

queue_t* queue_create(pthread_mutex_t* mutex, pthread_cond_t* not_empty) {
    queue_t* q = malloc(sizeof(queue_t));
    if (!q) {
        perror("queue_create: malloc failed");
        return NULL;
    }
    q->mutex = mutex;
    q->not_empty = not_empty;
    q->head = NULL;
    q->tail = NULL;
    q->count = 0;
    q->closed = 0;
    return q;
}

void queue_enqueue(queue_t* q, void* data) {
    pthread_mutex_lock(q->mutex);
    if (q->closed) {
        pthread_mutex_unlock(q->mutex);
        fprintf(stderr, "queue_enqueue: queue is closed, cannot enqueue\n");
        return;
    }
    node_t* new_node = malloc(sizeof(node_t));
    if (!new_node) {
        perror("queue_enqueue: malloc failed");
        pthread_mutex_unlock(q->mutex);
        return;
    }
    new_node->data = data;
    new_node->next = NULL;
    if (q->tail == NULL) {
        q->head = new_node;
        q->tail = new_node;
    } else {
        q->tail->next = new_node;
        q->tail = new_node;
    }
    q->count++;
    pthread_cond_signal(q->not_empty);
    pthread_mutex_unlock(q->mutex);
}

void* queue_dequeue(queue_t* q) {
    pthread_mutex_lock(q->mutex);
    while (q->head == NULL && !q->closed) {
        pthread_cond_wait(q->not_empty, q->mutex);
    }
    if (q->head == NULL && q->closed) {
        pthread_mutex_unlock(q->mutex);
        return NULL; // Queue is empty and closed
    }
    node_t* temp = q->head;
    void* data = temp->data;
    q->head = q->head->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    q->count--;
    pthread_mutex_unlock(q->mutex);
    free(temp);
    return data;
}

void queue_close(queue_t* q) {
    pthread_mutex_lock(q->mutex);
    q->closed = 1;
    pthread_cond_broadcast(q->not_empty); // Wake up any waiting threads
    pthread_mutex_unlock(q->mutex);
}

void queue_destroy(queue_t* q) {
    pthread_mutex_lock(q->mutex);
    node_t* current = q->head;
    while (current != NULL) {
        node_t* next = current->next;
        free(current);
        current = next;
    }
    q->head = NULL;
    q->tail = NULL;
    q->count = 0;
    q->closed = 1; // Mark as closed even if not explicitly closed
    pthread_mutex_unlock(q->mutex);
    free(q);
}