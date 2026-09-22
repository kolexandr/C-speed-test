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

#endif