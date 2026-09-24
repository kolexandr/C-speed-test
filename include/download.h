#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include "utils.h"

int download_test(const char *host, TransferStats *stats);

void print_download(TransferStats *stats);

#endif
