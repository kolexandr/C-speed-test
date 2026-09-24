#include <stdio.h>
#include <curl/curl.h>
#include <time.h>
#include <string.h>

#include "download.h"
#include "utils.h"

static const double DOWNLOAD_DURATION_SECONDS = 15;


static size_t write_callback(char *buffer, size_t size, size_t nmemb, void *userp){
    size_t received = size * nmemb;

    TransferStats *stats = userp;
    stats -> bytes += received;
    return received;
}

int download_test(const char *URL, TransferStats *stats){
    CURL *curl = curl_easy_init();

    if (curl == NULL){
        return -1;
    }

    // memset(stats, 0, sizeof(TransferStats));
    CURLcode result;

    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, stats);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "c-speedtest-cli/1.0");

    long response_code = 0;
    double local_start, total_time, elapsed, remaining;
    long remaining_ms;
    int count = 1;
    double start = get_time_seconds();

    while (1){
        elapsed = get_time_seconds() - start;

        if (elapsed >= DOWNLOAD_DURATION_SECONDS){
            break;
        }

        remaining = DOWNLOAD_DURATION_SECONDS - elapsed;
        remaining_ms = (long)(remaining * 1000);

        if (remaining_ms < 1) break;

        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, remaining_ms);
        
        
        local_start = get_time_seconds();
        result = curl_easy_perform(curl);
        
        if (result == CURLE_OPERATION_TIMEDOUT) break;

        if (result != CURLE_OK){
            printf("Download failed: %s\n", curl_easy_strerror(result));
            curl_easy_cleanup(curl);
            return -1;
        }

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        if (response_code < 200 || response_code >= 300){
            printf("Server returned HTTP %ld\n", response_code);
            curl_easy_cleanup(curl);
            return -1;
        }
      
        total_time = get_time_seconds() - local_start;
        printf("    Download No.%d\n", count);
        printf("    Downloaded time: %.3f seconds\n\n", total_time);
        // printf("    Downloaded: %lld bytes\n", (long long)stats.bytes);
        count++;
    }

    stats->total_time = get_time_seconds() - start;
    stats->mbps = calculate_mbps(stats);
    curl_easy_cleanup(curl);

    return 0;
}

void print_download(TransferStats *stats){
    printf("Download test has finished. RESULTS:\n");
    printf("Downloaded speed: %.2f Mbps/s\n", stats->mbps);
    printf("Downloaded time: %.3f seconds\n", stats->total_time);
    printf("Downloaded: %lld bytes\n", (long long)stats->bytes);
}