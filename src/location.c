#include <stdio.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <stdlib.h>
#include <string.h>

#include "location.h"
#include "utils.h"

#define API_LOCATION_URL "https://ipwho.is/"


static int parse_json_location(const char *json_string, Location *location ){
    cJSON *json_parsed = cJSON_Parse(json_string);

    if (json_parsed == NULL){
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL){
            fprintf(stderr, "Error: %s\n", error_ptr);
        }
        cJSON_Delete(json_parsed);
        return 1;
    }

    cJSON *country = cJSON_GetObjectItemCaseSensitive(json_parsed, "country");
    cJSON *city = cJSON_GetObjectItemCaseSensitive(json_parsed, "city");
    if ((cJSON_IsString(country)) && (country->valuestring != NULL)){
        snprintf(location->country, sizeof location->country, "%s", country->valuestring);
    }
    if ((cJSON_IsString(city)) && (city->valuestring != NULL)){
        snprintf(location->city, sizeof location->city, "%s", city->valuestring);
    }

    cJSON_Delete(json_parsed);
    return 0;
}

size_t write_callback(void *incoming, size_t size, size_t nmemb, void *userp){
    size_t total_size = size * nmemb;
    Buffer *buffer = (Buffer *)userp;

    char *temp_pointer = realloc(buffer->data, buffer->size + total_size + 1);
    buffer->data = temp_pointer;
    memcpy(buffer->data + buffer->size, incoming, total_size);
    buffer->size += total_size;
    buffer->data[buffer->size] = '\0';
    return total_size;
}

int get_location(Location *location){
    if (!location) return -1;
    
    CURL *curl = curl_easy_init();

    if (curl == NULL){
        return -1;
    }

    Buffer buffer;
    buffer_allocate(&buffer);


    curl_easy_setopt(curl, CURLOPT_URL, API_LOCATION_URL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);  
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);    
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "c-speedtest-cli/1.0");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode result = curl_easy_perform(curl);

    int found_location = -1;

    if (result != CURLE_OK){
        fprintf(stderr, "HTTP request failed %s\n", curl_easy_strerror(result));
    } else {
        long response_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        if (response_code != 200){
            fprintf(stderr, "HTTTP status error %ld\n", response_code);
            return found_location;
        } else {
            found_location = parse_json_location(buffer.data, location);
        }
    }

    return found_location;
}

void print_location(const Location *location){
    if (location->country[0] == '\0' || location->city[0] == '\0') return;
    printf("Location has been found:\n");
    printf("    Country : %s\n", location->country);
    printf("    City    : %s\n", location->city);
}