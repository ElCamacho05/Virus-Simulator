#include "Person.h"

// General Libraries
#include <stdlib.h>
#include <string.h>

// Other Classes libraries
#include "Regions.h"
#include "../Algorithms/Algorithms.h"

// PERSON variables
int PopulationCount = 0;

/*
---------------
PERSON Functions
---------------
*/

// ------------------
// Basic Functions
PERSON *createPerson(int id, char *name, int regionID, double initialDegree, double initialRisk, int daysInfected) {
    PERSON *nP =(PERSON*) malloc(sizeof(PERSON));
    if (nP == NULL) return NULL;
    
    nP->id = id;
    strcpy(nP->name, name);
    
    nP->regionID = regionID; 
    nP->initialDegree = initialDegree;
    nP->initialRisk = initialRisk;
    nP->daysInfected = daysInfected;

    nP->status = HEALTH;
    nP->actualStrainID = -1;
    nP->infectedBy = -1;

    P_DRAW_UTILS dC = {{0.0, 0.0}};
    nP->drawConf = dC;

    return nP;
};

// ------------------
// For Hash Functions
PERSON_HASH_TABLE* createPersonHashTable() {
    PERSON_HASH_TABLE *ht = (PERSON_HASH_TABLE*)calloc(1, sizeof(PERSON_HASH_TABLE));
    return ht;
}

void insertPersonInHash(PERSON_HASH_TABLE *ht, const PERSON *person) {
    if (!ht || !person) return;

    unsigned int index = hashFunction(person->id, PERSON_HASH_TABLE_SIZE);
    PERSON_NODE *new_node = (PERSON_NODE*)malloc(sizeof(PERSON_NODE));
    if (!new_node) return;

    new_node->data = *person;
    new_node->next = ht->table[index];
    ht->table[index] = new_node;
    ht->count++;
}

PERSON* searchPersonInHash(PERSON_HASH_TABLE *ht, int person_id) {
    if (!ht) return NULL;

    unsigned int index = hashFunction(person_id, PERSON_HASH_TABLE_SIZE);
    PERSON_NODE *current = ht->table[index];

    while (current != NULL) {
        if (current->data.id == person_id) {
            return &current->data;
        }
        current = current->next;
    }
    return NULL;
}

void freePersonInHash(PERSON_HASH_TABLE *ht) {
    if (!ht) return;
    for (int i = 0; i < PERSON_HASH_TABLE_SIZE; i++) {
        PERSON_NODE *current = ht->table[i];
        while (current != NULL) {
            PERSON_NODE *temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(ht);
}
