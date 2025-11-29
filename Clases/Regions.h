// Clases/Regions.h

#ifndef REGIONS_H
#define REGIONS_H

#include <string.h>

#define MAX_REGIONS 30 

// REGION structures
typedef struct Region{
    int id;
    char name[20];
    int infected;
    
    // AGREGADO: Para Tarea 2, necesitamos saber qué individuos están aquí.
    int *individuos_ids;
    int count_individuos;
    
} REGION;

// Regions variables
int RegionsCount = 0;

// REGION functions
REGION *createRegion(int id, char name[]);

#endif