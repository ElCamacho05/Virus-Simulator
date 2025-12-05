#ifndef PERSON_H
#define PERSON_H

#include "Regions.h" 
#include "Virus.h"

#define MAX_POPULATION 200 
#define PERSON_HASH_TABLE_SIZE 263

// --- CHANGE: Limit for Static Graph ---
#define MAX_CONTACTS 20 

// Defines the possible health states of an individual
typedef enum {
    HEALTH,
    INFECTED,
    IMMUNE,
    DEATH,
    ISOLATED,
    VACCINATED,
    QUARANTINE
} HealthStatus;

// Drawing utilities (ex, position in a graphical representation)
typedef struct PersonDrawUtils {
    double pos[2];
} P_DRAW_UTILS;

// Information about a single contact/edge in the static graph
typedef struct {
    int contactID;
    double weight; // 0.0 to 1.0 (0.1 = distant, 0.9 = very close/family)
} ContactInfo;

// Main structure representing an individual in the simulation
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

    // --- CHANGE: STATIC GRAPH (Array of IDs) ---
    // Stores contact information (ID and weight)
    ContactInfo contacts[MAX_CONTACTS];
    int numContacts;

    P_DRAW_UTILS drawConf;
} PERSON;

// Node structure for the Person Hash Table (linked list for collisions)
typedef struct PersonNode { 
    PERSON data;
    struct PersonNode *next;
} PERSON_NODE;

// Hash Table structure for efficient Person lookup
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