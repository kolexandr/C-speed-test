#include <stdio.h>
#include <curl/curl.h>
#include "download.h"

int main(){
    download_test("http://example.com");

    return 0;
}
