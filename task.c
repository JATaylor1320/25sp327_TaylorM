// task.c
#include "task.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#ifdef __linux__
#include <sys/types.h>
#include <unistd.h>
#endif

// Mutex for log_message
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void write_log(void* arg) {
    char* message = (char*)arg;
    if (message != NULL) {
        log_message(message); // Use the logging function
        free(message);
    }
}

void compute_sum(void* arg) {
    int* numbers = (int*)arg;
    if (numbers != NULL) {
        int sum = numbers[0] + numbers[1];
        char log_buffer[64];
        snprintf(log_buffer, sizeof(log_buffer), "SUM: %d + %d = %d", numbers[0], numbers[1], sum);
        log_message(log_buffer);
        free(numbers);
    }
}

void delay_task(void* arg) {
    int* delay = (int*)arg;
    if (delay != NULL) {
        char log_buffer[64];
        snprintf(log_buffer, sizeof(log_buffer), "Delaying for %d seconds...", *delay);
        log_message(log_buffer);
        sleep(*delay);
        log_message("Delay finished.");
        free(delay);
    }
}

void log_message(const char* message) {
    pthread_mutex_lock(&log_mutex);
    FILE *f = fopen("task.log.txt", "a");
    if (f) {
        time_t t = time(NULL);
        char timebuf[64];
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", localtime(&t));
#ifdef __linux__
        fprintf(f, "[%s] [%ld] %s\n", timebuf, (long)gettid(), message);
#else
        fprintf(f, "[%s] [?] %s\n", timebuf, message);
#endif
        fclose(f);
    }
    pthread_mutex_unlock(&log_mutex);
}