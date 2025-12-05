#include "Person.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../Algorithms/Algorithms.h"

int PopulationCount = 0;

PERSON *createPerson(int id, char *name, int regionID, double initialDegree, double initialRisk, int daysInfected) {
    PERSON *nP =(PERSON*) malloc(sizeof(PERSON));
    if (nP == NULL) return NULL;
    
    nP->id = id;
    strncpy(nP->name, name, 29);
    nP->name[29] = '\0';
    
    nP->regionID = regionID; 
    nP->initialDegree = initialDegree;
    nP->initialRisk = initialRisk;
    nP->daysInfected = daysInfected;

    nP->status = HEALTH;
    nP->actualStrainID = -1;
    nP->infectedBy = -1;

    // --- INITIALIZE STATIC GRAPH ---
    nP->numContacts = 0;
    for(int i=0; i<MAX_CONTACTS; i++) {
        nP->contacts[i].contactID = -1;
        nP->contacts[i].weight = 0.0;
    }

    P_DRAW_UTILS dC = {{0.0, 0.0}};
    nP->drawConf = dC;

    return nP;
};

// --- STATIC GRAPH LOGIC ---
void addContact(PERSON *p, int contactID, double weight) {
    if (!p) return;

    if (p->numContacts >= MAX_CONTACTS) return;

    // Check for duplicates
    for(int i=0; i<p->numContacts; i++) {
        if(p->contacts[i].contactID == contactID) return;
    }

    // Add with WEIGHT
    p->contacts[p->numContacts].contactID = contactID;
    p->contacts[p->numContacts].weight = weight; // Store closeness/likelihood
    p->numContacts++;
}

// ------------------
// For Hash Functions
PERSON_HASH_TABLE* createPersonHashTable() {
    PERSON_HASH_TABLE *ht = (PERSON_HASH_TABLE*)calloc(1, sizeof(PERSON_HASH_TABLE));
    return ht;
}

// Inserts a Person structure into the Hash Table using the person's ID as the key
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

// Searches for a person by their ID in the Hash Table
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

// Frees all memory allocated for the Hash Table and its nodes
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