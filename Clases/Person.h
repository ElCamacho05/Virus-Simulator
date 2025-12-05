#ifndef PERSON_H
#define PERSON_H

#include "Regions.h" 
#include "Virus.h"

#define MAX_POPULATION 200 
#define PERSON_HASH_TABLE_SIZE 263

// --- CAMBIO: Límite para Grafo Estático ---
#define MAX_CONTACTS 20 

typedef enum {
    HEALTH,
    INFECTED,
    IMMUNE,
    DEATH,
    ISOLATED
} HealthStatus;

typedef struct PersonDrawUtils {
    double pos[2];
} P_DRAW_UTILS;

typedef struct {
    int contactID;
    double weight; // 0.0 a 1.0 (0.1 = lejano, 0.9 = muy cercano/familia)
} ContactInfo;

typedef struct Person {
    int id;
    char name[30];
    int regionID; 
    double initialDegree; 
    double initialRisk; 
    HealthStatus status; 
    int actualStrainID;
    int daysInfected;
    int infectedBy;

    // --- CAMBIO: GRAFO ESTÁTICO (Array de IDs) ---
    // Ya no usamos punteros, guardamos directamente el ID del amigo
    ContactInfo contacts[MAX_CONTACTS];
    int numContacts;

    P_DRAW_UTILS drawConf;
} PERSON;

typedef struct PersonNode { 
    PERSON data;
    struct PersonNode *next;
} PERSON_NODE;

typedef struct { 
    PERSON_NODE *table[PERSON_HASH_TABLE_SIZE];
    int count;
} PERSON_HASH_TABLE;

extern int PopulationCount;

/*
---------------
PERSON Functions
---------------
*/

// ------------------
// Basic Functions
PERSON *createPerson(int id, char *name, int regionID, double initialDegree, double initialRisk, int daysInfected);
void addContact(PERSON *p, int contactID, double weight);

// ------------------
// For Hash Functions
PERSON_HASH_TABLE* createPersonHashTable();
void insertPersonInHash(PERSON_HASH_TABLE *ht, const PERSON *person);
PERSON* searchPersonInHash(PERSON_HASH_TABLE *ht, int person_id);
void freePersonInHash(PERSON_HASH_TABLE *ht);

#endif