#ifndef SERVER_H
#define SERVER_H

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

int find_best_server(const ServerList *list, const Location *location, Server *server);

void print_server(Server *server);

void free_server_list(ServerList *list);

#endif