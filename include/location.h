#ifndef LOCATION_H
#define LOCATION_H

typedef struct{
    char city[128];
    char country[128];
} Location;

int get_location(Location *location);

void print_location(const Location *location);

#endif