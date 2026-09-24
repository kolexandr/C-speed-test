#include <stdio.h>
#include <time.h>
#include <curl/curl.h>
#include <stdlib.h>

#include "utils.h"

void buffer_allocate(Buffer *buffer){
    buffer->data = malloc(1);
    buffer->size = 0;
    if (buffer->data == NULL){
        buffer->data[0] = '\0';
    }
}

void buffer_free(Buffer *buffer){
    free(buffer->data);
    buffer->data = NULL;
    buffer->size = 0;
}

double calculate_mbps(TransferStats *stats){
    if (stats->total_time <= 0) return 0;
    return ((double)stats->bytes * 8) / (1000000.0 * stats->total_time);
}

double get_time_seconds(){
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}
