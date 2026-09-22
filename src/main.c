#include <stdio.h>
#include <curl/curl.h>
#include <unistd.h>

#include "download.h"
#include "upload.h"
#include "location.h"
#include "server.h"

#define SERVER_JSON "speedtest_server_list.json"
void print_manual(void){
    printf("a - automatic test, d - download test, u - upload test, l - location\n");
}


int main(int argc, char *argv[]){

    int auto_mode = 0;
    int download_mode = 0;
    int upload_mode = 0;
    int location_mode = 0;
    int server_mode = 0;

    int option;

    while((option = getopt(argc, argv, "aduhl")) != -1){
        switch(option){
            case 'a':
                auto_mode = 1;
                break;
            case 'd':
                // if(download_test("http://speedtest.litnet.lt:8080/speedtest/random4000x4000.jpg") == 1){
                //     return 1;
                // };
                download_mode = 1;
                break;
            case 'h':
                print_manual();
                break;
            case 'l':
                location_mode = 1;
                break;
            default: // work only when there is a wrong flag. To do: build handler if there is no flag
                printf("No flag specified for the program. Use -h for help.\n");
                return 1;
        }
    }

    if (download_mode){
        download_test("http://speedtest.litnet.lt:8080/speedtest/random4000x4000.jpg");
    }

    if (location_mode){
        Location location;
        if (get_location(&location) == 0){
            print_location(&location);
        } else {
            fprintf(stderr, "Location is unavailable at the moment. Please try later.\n");
        }
    }

    // global curl is needed

    return 0;
}
