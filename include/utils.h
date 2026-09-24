#ifndef UTILS_H
#define UTILS_H

typedef struct{
    char *data;
    size_t size;
} Buffer;

typedef struct {
    double mbps;
    double total_time;
    curl_off_t bytes;
} TransferStats;

void buffer_allocate(Buffer *buffer);

void buffer_free(Buffer *buffer);

double calculate_mbps(TransferStats *stats);

double get_time_seconds();

#endif