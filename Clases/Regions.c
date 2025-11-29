// General Libraries
#include <stdlib.h>

// Self Library
#include "Regions.h"

// REGION functions
REGION *createRegion(int id, char name[]) {
    REGION *reg = (REGION *) malloc(sizeof(REGION));
    reg->id = id;
    strcpy(reg->name, name);
    reg->infected = 0;
    return reg;
}