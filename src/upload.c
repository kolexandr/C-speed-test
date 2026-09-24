#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>

#include "upload.h"
#include "utils.h" 

#define UPLOAD_BUFFER_SIZE (1024 * 1024)
const static double test_duration_seconds = 15;

typedef struct {
    unsigned char *data;
    size_t data_size;
    size_t off_set;

    curl_off_t bytes_sent;
    curl_off_t total_bytes;
} UploadData;



static size_t read_callback(char *buffer, size_t size, size_t nmemb, void *userp){
    UploadData *upload = userp;

    size_t capacity = size * nmemb;

    curl_off_t remaining = upload->total_bytes - upload->bytes_sent;

    if (remaining = 0){
        return 0;
    }

    curl_off_t bytes_to_send = remaining < capacity ? remaining : capacity;

    curl_off_t copied = 0;

    
}

static size_t write_callback(char *buffer, size_t size, size_t nmemb, void *userp){

}


double upload_test(const char *url, TransferStats *stats){

    UploadData upload = {0};
    upload.data = malloc(UPLOAD_BUFFER_SIZE);

    if (upload.data == NULL){
        return 1;
    }

    upload.data_size = UPLOAD_BUFFER_SIZE;
    upload.off_set = 0;

    CURL *curl = curl_easy_init();

    if(curl == NULL){
        free(upload.data);
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    // curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload);
    // curl_easy_setopt(curl, CURLOPT_WRITEDATA, &stats);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "c-speedtest-cli/1.0");
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    

    CURLcode result;




    curl_easy_cleanup(curl);
    free(upload.data);

    return 0;
}