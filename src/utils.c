#include <stdio.h>
#include <time.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

void buffer_allocate(Buffer *buffer){
    buffer->data = malloc(1);
    buffer->size = 0;
    if (buffer->data != NULL){
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

double get_time_seconds(void){
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}

int build_url(char *url, size_t url_size, const char *host, const char *path){
    if (url == NULL || host == NULL || host[0] == '\0' || path == NULL || url_size == 0) {
        return -1;
    }

    const char *scheme = "http://";
    if (strncmp(host, "http://", 7) == 0 || strncmp(host, "https://", 8) == 0){
        scheme = "";
    } else if (strstr(host, "://") != NULL){
        return -1;
    }
    size_t length = strlen(host);
    while (length > 0 && host[length - 1] == '/') length--;
    if (length == 0 || length >= url_size) return -1;
    int written = snprintf(url, url_size, "%s%.*s%s", scheme, (int)length, host, path);

    if (written < 0 || (size_t)written >= url_size) {
        return -1;
    }

    return 0;
}
