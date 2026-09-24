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
    printf("    -d      <country>       Download test on a chosen country\n");
    printf("    -u      <country>       Upload test on a chosen country\n");
    printf("    -l                      Find current location and display\n");
    printf("    -s                      Find best server based on current location\n");
    printf("\n");
    printf("    -h                      Display manual\n");
}

void auto_action(ServerList *list, Server *server, Location *location){
    
}

int main(int argc, char *argv[]){

    int auto_mode = 0;
    int download_mode = 0;
    int upload_mode = 0;
    int location_mode = 0;
    int server_mode = 0;
    char *server_name = NULL;

    int option;

    if (argc == 1) {
        fprintf(stderr, "No flag specified. Use -h for help.\n");
        return 1;
    }

    while((option = getopt(argc, argv, "ad:u:lsh")) != -1){ // add values :
        switch(option){
            case 'a':
                auto_mode = 1;
                break;
            case 'd':
                download_mode = 1;
                server_name = optarg;
                break;
            case 'u':
                upload_mode = 1;
                server_name = optarg;
                break;
            case 'h':
                print_manual();
                break;
            case 'l':
                location_mode = 1;
                break;
            case 's':
                server_mode = 1;
                break;
            default: // work only when there is a wrong flag. To do: build handler if there is no flag
                fprintf(stderr, "Invalid option specified for the program. Use -h for help.\n");
                return 1;
        }
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    if (download_mode){
        if (server_name == NULL){
            fprintf(stderr, "Download test requires a server. Please specify a server: -d <server>");
            return 1;
        }
        TransferStats stats = {0};
        if(download_test(server_name, &stats)){
            print_download(&stats);
        }
        
    }

    if (upload_mode){
        if (server_name == NULL){
            fprintf(stderr, "Upload test requires a server. Please specify a server: -u <server>");
        }
        TransferStats stats = {0};
        if(upload_test(server_name, &stats)){
            print_download(&stats); //change to upload
        }
    }

    if (location_mode){
        Location location;
        if (get_location(&location) == 0){
            print_location(&location);
        } else {
            fprintf(stderr, "Location is unavailable at the moment. Please try later.\n");
        }
    }

    if (server_mode){ //simplify
        Location location;
        if (get_location(&location) == 0){
            print_location(&location);
        } else {
            fprintf(stderr, "Location is unavailable at the moment. Please try later.\n");
        }
        ServerList list;
        Server server;
        if (load_server_list(&list, SERVER_JSON) != 0){
            fprintf(stderr, "Failed to load server list.\n");
        }

        int result = find_best_server(&list, &location, &server);
        free_server_list(&list);

        if (result != 0){
            fprintf(stderr, "No server avaliable for: %s %s", location.city, location.country);
        }

        print_server(&server);
    }

    curl_global_cleanup();

    return 0;
}
