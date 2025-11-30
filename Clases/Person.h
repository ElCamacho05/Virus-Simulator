// Clases/Person.h

#ifndef PERSON_H
#define PERSON_H

// Self Library
#include "Regions.h" 
#include "Virus.h"

// PERSON constants
#define MAX_POPULATION 200 
#define PERSON_HASH_TABLE_SIZE 263

// Actual state of the Person
typedef enum {
    HEALTH,
    INFECTED,
    IMMUNE,
    DEATH
} HealthStatus;

// PERSON structures

typedef struct PersonDrawUtils {
    double pos[2];
} P_DRAW_UTILS;

typedef struct Person{
    int id;
    char name[30];
    
    int regionID; 
    
    double initialDegree; 
    
    double initialRisk; 
    
    HealthStatus status; 
    int actualStrainID;
    int daysInfected;
    int infectedBy;

    P_DRAW_UTILS drawConf;
} PERSON;

typedef struct PersonNode { // HASH wrapper structure
    PERSON data;
    struct PersonNode *next;
} PERSON_NODE;

typedef struct { // HASH TABLE centralized structure
    PERSON_NODE *table[PERSON_HASH_TABLE_SIZE];
    int count;
} PERSON_HASH_TABLE;

// Person variables
extern int PopulationCount;

/*
---------------
PERSON Functions
---------------
*/

// ------------------
// Basic Functions
PERSON *createPerson(int id, char *name, int regionID, double initialDegree, double initialRisk, int daysInfected);

// ------------------
// For Hash Functions
PERSON_HASH_TABLE* createPersonHashTable();
void insertPersonInHash(PERSON_HASH_TABLE *ht, const PERSON *person);
PERSON* searchPersonInHash(PERSON_HASH_TABLE *ht, int person_id);
void freePersonInHash(PERSON_HASH_TABLE *ht);

#endif