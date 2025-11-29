#ifndef PERSON_H
#define PERSON_H

// Self Library
#include "Regions.h"

// PERSON constants
#define MAX_POPULATION 200

// PERSON structures
typedef struct Person{
    int id;
    char name[30];
    REGION *region;
    double initialDegree;
    double contagiousness;
    // ContList *directory;
    int daysInfected;
} PERSON;

// POPULATION variables
int PopulationCount = 0;

// PERSON functions
PERSON *createPerson(int id, char *name, REGION region, double initialDegree, double contagiousness, int daysInfected);

#endif
