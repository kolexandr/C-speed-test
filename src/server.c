#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <cjson/cJSON.h>
#include <curl/curl.h>

#include "utils.h"
#include "location.h"
#include "server.h"


static char *read_file(const char *json_file){
    FILE *file = fopen(json_file, "r");
    if (file == NULL){
        fprintf(stderr, "Unable to open the file.\n");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long len = ftell(file);

    rewind(file);
    if (len <= 0){
        fprintf(stderr, "No content in the file.");
        fclose(file);
        return NULL;
    }

    char *buffer = malloc((size_t)len + 1);
    if(buffer == NULL){
        fprintf(stderr, "Error allocating memory");
        fclose(file);
        return NULL;
    }

    size_t read = fread(buffer, 1, (size_t)len, file);
    buffer[read] = '\0';

    fclose(file);
    return buffer;
}

static void create_url(const char *host, char *url, size_t size){
    if (strncmp(host, "http", 4) == 0){
        snprintf(url, size, "%s", host);
    } else {
        snprintf(url, size, "http://%s", host);
    }
}

static int check_reachable(const char *host){
    char url[512];
    CURL *curl = curl_easy_init();

    if (curl == NULL){
        return 0;
    }
    create_url(host, url, sizeof(url));

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "c-speedtest-cli/1.0");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode result = curl_easy_perform(curl);

    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    curl_easy_cleanup(curl);

    return result == CURLE_OK && response_code >= 200 && response_code < 300;
}


int load_server_list(ServerList *list, const char *json_file){
    if (!list || !json_file) return -1;

    list->servers = NULL;
    list->count = 0;

    char *text = read_file(json_file);
    if (!text) return -1;

    cJSON *json_parsed = cJSON_Parse(text);
    free(text);
    if (!json_parsed){
        fprintf(stderr, "Failed to parse json.");
        return -1;
    }


    if (!cJSON_IsArray(json_parsed)) {
        fprintf(stderr, "server: JSON root is not an array");
        cJSON_Delete(json_parsed);
        return -1;
    }

    int total = cJSON_GetArraySize(json_parsed);
    if (total <= 0) {
        fprintf(stderr, "server: server list is empty");
        cJSON_Delete(json_parsed);
        return -1;
    }

    list->servers = calloc((size_t)total, sizeof(Server));
    if (!list->servers) {
        fprintf(stderr, "Out of memory allocating server list");
        cJSON_Delete(json_parsed);
        return -1;
    }

    int index = 0;
    cJSON *item = NULL;

    cJSON_ArrayForEach(item, json_parsed) {
        cJSON *country = cJSON_GetObjectItemCaseSensitive(item, "country");
        cJSON *city = cJSON_GetObjectItemCaseSensitive(item, "city");
        cJSON *provider = cJSON_GetObjectItemCaseSensitive(item, "provider");
        cJSON *host = cJSON_GetObjectItemCaseSensitive(item, "host");

        if (!cJSON_IsString(host) || !host->valuestring) continue;

        Server *server = &list->servers[index];

        snprintf(server->host, 256, "%s", host->valuestring);
        snprintf(server->country, 128, "%s", cJSON_IsString(country) ? country->valuestring : "Unknown");
        snprintf(server->city, 128, "%s", cJSON_IsString(city) ? city->valuestring : "Unknown");
        snprintf(server->provider, 128, "%s", cJSON_IsString(provider) ? provider->valuestring : "Unknown");

        index++;
    }

    list->count = index;
    cJSON_Delete(json_parsed);

    return 0;
}

int find_server_by_country(const ServerList *list, const char *country, Server *server){
    if (!list || !list->servers || !country || !country[0] || !server){
        return -1;
    }

    for (int i = 0; i < list->count; i++){
        if (strcasecmp(list->servers[i].country, country) == 0 &&
            check_reachable(list->servers[i].host)){
            *server = list->servers[i];
            return 0;
        }
    }

    fprintf(stderr, "No reachable server found for country: %s\n", country);
    return -1;
}

int find_best_server(const ServerList *list, const Location *location, Server *server){
    if (!list || !list->servers || !location || !server){
        return -1;
    }

    for (int i = 0; i < list->count; i++){
        if (strcasecmp(list->servers[i].country, location->country) == 0 &&
            strcasecmp(list->servers[i].city, location->city) == 0 &&
            check_reachable(list->servers[i].host)){
            *server = list->servers[i];
            return 0;
        }
    }

    return find_server_by_country(list, location->country, server);
}


void free_server_list(ServerList *list){
    free(list->servers);
    list->servers = NULL;
    list->count = 0;
}

void print_server(Server *server){
    if (!server) return;
    printf("Server was detected!:\n");
    printf("    Country :%s\n", server->country);
    printf("    City    :%s\n", server->city);
    printf("    Provider:%s\n", server->provider);
    printf("    Host    :%s\n\n", server->host);
}