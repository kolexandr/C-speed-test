#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "upload.h"
#include "utils.h"

#define UPLOAD_BUFFER_SIZE (1024 * 1024)
#ifndef UPLOAD_DURATION_SECONDS
#define UPLOAD_DURATION_SECONDS 15
#endif
#define UPLOAD_PATH "/speedtest/upload.php"
#define URL_SIZE 512

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


static size_t discard_response(char *data, size_t size, size_t nmemb, void *userp){
    (void)data;
    (void)userp;
    return size * nmemb;
}

int upload_test(const char *host, TransferStats *stats){
    if (!stats) return -1;
    memset(stats, 0, sizeof(*stats));
    char url[URL_SIZE];
    if (build_url(url, sizeof(url), host, UPLOAD_PATH) != 0){
        fprintf(stderr, "Invalid upload host.\n");
        return -1;
    }

    unsigned char *buffer = malloc(UPLOAD_BUFFER_SIZE);
    if (!buffer){
        fprintf(stderr, "Failed to allocate upload buffer.\n");
        return -1;
    }
    fill_test_buffer(buffer, UPLOAD_BUFFER_SIZE);
    CURL *curl = curl_easy_init();
    if (!curl){
        free(buffer);
        return -1;
    }
    struct curl_slist *headers = curl_slist_append(NULL, "Content-Type: application/octet-stream");
    if (!headers){
        curl_easy_cleanup(curl);
        free(buffer);
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (char *)buffer);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)UPLOAD_BUFFER_SIZE);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard_response);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 5000L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "c-speedtest-cli/1.0");

    double start = get_time_seconds();
    int status = -1;
    while (1){
        double remaining = UPLOAD_DURATION_SECONDS - (get_time_seconds() - start);
        long remaining_ms = (long)(remaining * 1000);
        if (remaining_ms < 1){
            status = stats->bytes > 0 ? 0 : -1;
            break;
        }
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, remaining_ms);
        CURLcode result = curl_easy_perform(curl);
        long response_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        if (result == CURLE_OPERATION_TIMEDOUT &&
            get_time_seconds() - start >= UPLOAD_DURATION_SECONDS - 0.01 &&
            (response_code == 0 || (response_code >= 200 && response_code < 300))){
            status = stats->bytes > 0 ? 0 : -1;
            if (status != 0) fprintf(stderr, "Upload timed out without a confirmed transfer.\n");
            break;
        }
        if (result != CURLE_OK || response_code < 200 || response_code >= 300){
            fprintf(stderr, "Upload failed: HTTP %ld (%s).\n", response_code, curl_easy_strerror(result));
            break;
        }
        curl_off_t bytes = 0;
        curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD_T, &bytes);
        if (bytes != UPLOAD_BUFFER_SIZE){
            fprintf(stderr, "Server did not accept the complete upload.\n");
            break;
        }
        stats->bytes += bytes;
    }

    stats->total_time = get_time_seconds() - start;
    if (status == 0) stats->mbps = calculate_mbps(stats);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    free(buffer);
    return status;
}

void print_upload(TransferStats *stats){
    printf("Upload test has finished. RESULTS:\n");
    printf("Upload speed: %.2f Mbps\n", stats->mbps);
    printf("Upload time: %.3f seconds\n", stats->total_time);
    printf("Uploaded: %lld bytes\n\n", (long long)stats->bytes);
}