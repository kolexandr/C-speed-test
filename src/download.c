#include <stdio.h>
#include <curl/curl.h>
#include <time.h>
#include <string.h>
#include <strings.h>

#include "download.h"
#include "utils.h"

#ifndef DOWNLOAD_DURATION_SECONDS
#define DOWNLOAD_DURATION_SECONDS 15
#endif
#define DOWNLOAD_PATH "/speedtest/random4000x4000.jpg"
#define URL_SIZE 512


static size_t write_callback(char *buffer, size_t size, size_t nmemb, void *userp){
    (void)buffer;
    size_t received = size * nmemb;

    TransferStats *stats = userp;
    stats -> bytes += received;
    return received;
}

int download_test(const char *host, TransferStats *stats){
    if (!stats) return -1;
    memset(stats, 0, sizeof(*stats));
    char url[URL_SIZE];

    if (build_url(url, sizeof(url), host, DOWNLOAD_PATH) != 0) {
        return -1;
    }

    CURL *curl = curl_easy_init();

    if (curl == NULL){
        return -1;
    }

    CURLcode result;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, stats);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 5000L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "c-speedtest-cli/1.0");

    long response_code = 0;
    double elapsed, remaining;
    long remaining_ms;
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
        
        
        curl_off_t previous_bytes = stats->bytes;
        result = curl_easy_perform(curl);
        


        if (result != CURLE_OK && result != CURLE_OPERATION_TIMEDOUT){
            printf("Download failed: %s\n", curl_easy_strerror(result));
            curl_easy_cleanup(curl);
            return -1;
        }

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        if (result == CURLE_OPERATION_TIMEDOUT && response_code == 0 &&
            previous_bytes > 0 &&
            get_time_seconds() - start >= DOWNLOAD_DURATION_SECONDS - 0.01){
            break;
        }
        if (response_code < 200 || response_code >= 300){
            printf("Server returned HTTP %ld\n", response_code);
            curl_easy_cleanup(curl);
            return -1;
        }
      
        char *content_type = NULL;
        curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content_type);
        if ((content_type && strncasecmp(content_type, "text/html", 9) == 0) ||
            stats->bytes == 0 ||
            (result == CURLE_OK && stats->bytes == previous_bytes)){
            fprintf(stderr, "Download endpoint returned no test data or an HTML page.\n");
            curl_easy_cleanup(curl);
            return -1;
        }
        if (result == CURLE_OPERATION_TIMEDOUT){
            if (get_time_seconds() - start < DOWNLOAD_DURATION_SECONDS - 0.01){
                fprintf(stderr, "Download connection timed out.\n");
                curl_easy_cleanup(curl);
                return -1;
            }
            break;
        }
    }

    stats->total_time = get_time_seconds() - start;
    stats->mbps = calculate_mbps(stats);
    curl_easy_cleanup(curl);

    return stats->bytes > 0 ? 0 : -1;
}

void print_download(TransferStats *stats){
    printf("Download test has finished. RESULTS:\n");
    printf("    Download speed:      %.2f Mbps\n", stats->mbps);
    printf("    Downloaded time:     %.3f seconds\n", stats->total_time);
    printf("    Downloaded:          %lld bytes\n\n", (long long)stats->bytes);
}