#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <stdint.h>

#include "upload.h"
#include "utils.h" 

#define UPLOAD_BUFFER_SIZE (1024 * 1024)
#define TEST_DURATION_SECONDS 15
#define TEST_DURATION_MS 15000L

typedef struct {
    unsigned char *buffer;
    size_t buffer_size;
    size_t offset;


    double start_time;
} UploadData;

static uint32_t xorshift32(uint32_t *state)
{
    uint32_t x = *state;

    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;

    *state = x;

    return x;
}

static void fill_test_buffer(unsigned char *buffer, size_t size)
{
    uint32_t state = 0x12345678;

    for (size_t i = 0; i < size; i++) {
        buffer[i] = (unsigned char)xorshift32(&state);
    }
}

static size_t read_callback(char *content, size_t size, size_t nmemb, void *userp){
    UploadData *upload = userp;

    size_t capacity = size * nmemb;

    size_t copied = 0;

    while(copied < capacity){
        size_t available = upload->buffer_size - upload->offset;
        size_t amount = capacity - copied;

        if (amount > available){
            amount = available;
        }

        memcpy(content + copied, upload->buffer + upload->offset, amount);

        copied += amount;
        upload->offset += amount;

        if (upload->offset == upload->buffer_size) {
            upload->offset = 0;
        }
    }
    
    return copied;
    
}

static int progress_callback(void *content, void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow){
    (void)dltotal;
    (void)dlnow;
    (void)ultotal;
    (void)ulnow;

    UploadData *upload = clientp;

    double elapsed =
        get_time_seconds() - upload->start_time;

    if (elapsed >= TEST_DURATION_SECONDS) {
        return 1;
    }

    return 0;
}


int upload_test(const char *url, TransferStats *stats){

    UploadData upload = {0};
    upload.buffer = malloc(UPLOAD_BUFFER_SIZE);

    if (upload.buffer == NULL){
        fprintf(stderr, "Failed to allocate upload buffer.\n");
        return 1;
    }

    upload.buffer_size = UPLOAD_BUFFER_SIZE;
    upload.offset = 0;

    CURL *curl = curl_easy_init();

    if(curl == NULL){
        free(upload.buffer);
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &upload);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, TEST_DURATION_MS);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "c-speedtest-cli/1.0");
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    upload.start_time = get_time_seconds();
    

    CURLcode result = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD_T, stats->bytes);
    stats->total_time = get_time_seconds() - upload.start_time;
    stats->mbps = calculate_mbps(stats);

    int success = 0;

    if (result != CURLE_OK &&
        result != CURLE_ABORTED_BY_CALLBACK &&
        result != CURLE_OPERATION_TIMEDOUT) {

        fprintf(
            stderr,
            "Upload failed: %s\n",
            curl_easy_strerror(result)
        );

        success = -1;
    }


    curl_easy_cleanup(curl);
    free(upload.buffer);

    return success;
}