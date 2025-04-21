#ifndef TASK_H
#define TASK_H

#include "queue.h"

typedef struct request_s {
    void (*work_fn)(void* arg);
    void* arg;
};
typedef struct request_s request_t;
void write_log(void* arg);
void compute_sum(void* arg);
void delay_task(void* arg);
void log_message(const char* message);

#endif