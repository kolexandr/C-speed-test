#include <stdio.h>
#include <curl/curl.h>
#include "download.h"
#include "upload.h"

int main(int argc, char argv[]){
    download_test("http://speedtest.litnet.lt:8080/speedtest/random4000x4000.jpg");

    return 0;
}
