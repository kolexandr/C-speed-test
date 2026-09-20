#include <stdio.h>
#include <curl/curl.h>
#include "upload.h"

double upload_test(const char *URL){
    CURL *curl = curl_easy_init();

    if(curl == NULL){
        return 1;
    }

    // curl_easy_opt(curl, );

    curl_easy_cleanup(curl);

    return 0;
}