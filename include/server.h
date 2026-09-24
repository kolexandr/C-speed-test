#ifndef SERVER_H
#define SERVER_H

#include "location.h"

typedef struct{
    char country[128];
    char city[128];
    char provider[128];
    char host[256];
} Server;

typedef struct{
    Server *servers;
    int count;
} ServerList;

int load_server_list(ServerList *list, const char *json_file);

// Returns 0 and copies the first reachable match into server; -1 on failure.
int find_server_by_country(const ServerList *list, const char *country, Server *server);

int find_best_server(const ServerList *list, const Location *location, Server *server);

void print_server(Server *server);

void free_server_list(ServerList *list);

#endif