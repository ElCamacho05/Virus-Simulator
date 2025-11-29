#ifndef REGIONS_H
#define REGIONS_H

#include <string.h>

#define MAX_REGIONS 30 // 20 ORIGINALLY

// REGION structures
typedef struct Region{
    int id;
    char name[20];
    int infected;
} REGION;

// Regions variables
int RegionsCount = 0;

// REGION functions
REGION *createRegion(int id, char name[]);

#endif