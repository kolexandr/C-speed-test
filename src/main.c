#include <stdio.h>
#include <curl/curl.h>
#include <unistd.h>

#include "download.h"
#include "upload.h"
#include "location.h"
#include "server.h"
#include "utils.h"

#define SERVER_JSON "speedtest_server_list.json"

void print_manual(void){
    printf("    -a                      Automatic test\n");
    printf("    -d      <country>       Download test in a chosen country\n");
    printf("    -u      <country>       Upload test in a chosen country\n");
    printf("    -l                      Find current location and display\n");
    printf("    -s                      Find best server based on current location\n");
    printf("\n");
    printf("    -h                      Display manual\n");
}

static int download_action(const char *url){
    TransferStats stats = {0};
    if (download_test(url, &stats) != 0){
        return 1;
    }

    print_download(&stats);
    return 0;
}

static int upload_action(const char *url){
    TransferStats stats = {0};
    if (upload_test(url, &stats) != 0){
        return 1;
    }

    print_upload(&stats);
    return 0;
}

static int location_action(Location *location){
    if (get_location(location) != 0){
        fprintf(stderr, "Location is unavailable at the moment. Please try later.\n");
        return 1;
    }

    print_location(location);
    return 0;
}

static int server_action(Server *server){
    Location location = {0};
    if (location_action(&location) != 0){
        return 1;
    }

    ServerList list = {0};
    if (load_server_list(&list, SERVER_JSON) != 0){
        fprintf(stderr, "Failed to load server list.\n");
        return 1;
    }

    int result = find_best_server(&list, &location, server);
    free_server_list(&list);
    if (result != 0){
        fprintf(stderr, "No server available for: %s %s\n", location.city, location.country);
        return 1;
    }

    print_server(server);
    return 0;
}

static int country_action(const char *country, int upload_mode){
    ServerList list = {0};
    Server server = {0};
    if (load_server_list(&list, SERVER_JSON) != 0){
        fprintf(stderr, "Failed to load server list.\n");
        return 1;
    }

    int result = find_server_by_country(&list, country, &server);
    free_server_list(&list);
    if (result != 0) return 1;

    print_server(&server);
    return upload_mode ? upload_action(server.host) : download_action(server.host);
}

static int auto_action(void){
    Server server = {0};
    if (server_action(&server) != 0){
        return 1;
    }

    int status = download_action(server.host);
    status |= upload_action(server.host);
    return status;
}

int main(int argc, char *argv[]){

    int auto_mode = 0;
    int download_mode = 0;
    int upload_mode = 0;
    int location_mode = 0;
    int server_mode = 0;
    int help_mode = 0;
    const char *download_country = NULL;
    const char *upload_country = NULL;

    int option;

    if (argc == 1) {
        fprintf(stderr, "No flag specified. Use -h for help.\n");
        return 1;
    }

    while((option = getopt(argc, argv, "ad:u:lsh")) != -1){
        switch(option){
            case 'a':
                auto_mode = 1;
                break;
            case 'd':
                download_mode = 1;
                download_country = optarg;
                break;
            case 'u':
                upload_mode = 1;
                upload_country = optarg;
                break;
            case 'h':
                help_mode = 1;
                break;
            case 'l':
                location_mode = 1;
                break;
            case 's':
                server_mode = 1;
                break;
            default:
                fprintf(stderr, "Invalid option specified for the program. Use -h for help.\n");
                return 1;
        }
    }

    if (optind < argc){
        fprintf(stderr, "Unexpected argument: %s. Use -h for help.\n", argv[optind]);
        return 1;
    }

    if (help_mode){
        print_manual();
    }

    if (!auto_mode && !download_mode && !upload_mode && !location_mode && !server_mode){
        if (help_mode) return 0;
        fprintf(stderr, "No flag specified. Use -h for help.\n");
        return 1;
    }

    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK){
        fprintf(stderr, "Failed to initialize curl.\n");
        return 1;
    }

    int status = 0;
    if (auto_mode){
        status |= auto_action();
    }
    if (download_mode){
        status |= country_action(download_country, 0);
    }
    if (upload_mode){
        status |= country_action(upload_country, 1);
    }
    if (location_mode){
        Location location = {0};
        status |= location_action(&location);
    }
    if (server_mode){
        Server server = {0};
        status |= server_action(&server);
    }

    curl_global_cleanup();
    return status;
}
