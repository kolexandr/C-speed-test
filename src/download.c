#include <stdio.h>
#include <curl/curl.h>
#include "download.h"

typedef struct {
    curl_off_t bytes;
} TransferStats;

static size_t write_callback(char *buffer, size_t size, size_t nmemb, void *userp){
    size_t received = size * nmemb;

    TransferStats *stats = userp;
    stats -> bytes += received;
    return received;
}

static double calculate_mbps(TransferStats *stats, double seconds){
    if (seconds <= 0) return 0;
    return ((double)stats->bytes * 8) / (1000000.0 * seconds);
}

double download_test(const char *URL){
    CURL *curl = curl_easy_init();

    if (curl == NULL){
        return -1;
    }

    TransferStats stats = {0};
    CURLcode result;
    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &stats);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // for developing
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/8.18.0");

    result = curl_easy_perform(curl);

    if (result != CURLE_OK && result != CURLE_OPERATION_TIMEDOUT){
        printf("Download failed: %s\n", curl_easy_strerror(result));
        curl_easy_cleanup(curl);
        return -1;
    }

    // for development
    long response_code = 0;

    curl_easy_getinfo(
        curl,
        CURLINFO_RESPONSE_CODE,
        &response_code
    );

    printf("HTTP status: %ld\n", response_code);
    printf("CURL result: %d - %s\n", result, curl_easy_strerror(result));

    double total_time = 0;

    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);
    
    double mbps = calculate_mbps(&stats, total_time);

    printf("Downloaded: %lld bytes\n", (long long)stats.bytes);
    printf("Downloaded time: %.3f seconds\n", total_time);
    printf("Downloaded speed: %.2f mbps/s\n", mbps);

    curl_easy_cleanup(curl);

    return mbps;
}