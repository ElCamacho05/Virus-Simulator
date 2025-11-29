#include "Person.h"
#include "Regions.h"
#include "Virus.h"

#include <stdlib.h>
#include <string.h>

// PERSON functions
PERSON *createPerson(int id, char *name, REGION region, double initialDegree, double contagiousness, int daysInfected) {
    PERSON *nP =(PERSON*) malloc(sizeof(PERSON));
    nP->id = id;
    strcpy(nP->name, name);
    nP->initialDegree = initialDegree;
    nP->contagiousness = contagiousness;
    nP->daysInfected = daysInfected;
    return nP;
};
