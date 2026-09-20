#include <stdio.h>
#include <curl/curl.h>
#include "download.h"

static size_t write_callback(char *buffer, size_t size, size_t nmemb, void *userp){
    size_t bytes = size * nmemb;
    printf("New chunk (%zu bytes)\n", bytes);
    return bytes;
}

double download_test(const char *URL){
    CURL *curl;

    curl = curl_easy_init();

    if (curl == NULL){
        return 1;
    }

    CURLcode result;
    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    result = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    

    return 0;
}