#ifndef QUEUE_INTERNAL_H
#define QUEUE_iNTERNAL_H

typedef struct node_s {
    void* data; 
    struct node_s* next;
} node_t;

#endif
