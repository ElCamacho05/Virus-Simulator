// Self Library
#include "Regions.h"

// General Libraries
#include <stdlib.h>
#include <string.h>

// Other Classes libraries
#include "Person.h"
#include "../Algorithms/Algorithms.h"

// REGION variables
int RegionsCount;

/*
---------------
REGION Functions
---------------
*/

// ---------------
// Basic Functions
// Allocates memory and initializes a new region structure
REGION *createRegion(int id, char name[]) {
    REGION *reg = (REGION *) malloc(sizeof(REGION));
    if (reg == NULL) return NULL;
    
    reg->id = id;
    strcpy(reg->name, name);
    reg->infectedCount = 0;
    
    reg->populationCount = 0;
    // Allocates memory to store IDs of people belonging to this region
    reg->peopleIDs = (int *) calloc(MAX_POPULATION, sizeof(int)); 

    reg->numConnections = 0;
    // Initialize static connections array
    for(int i=0; i<MAX_REGION_CONNECTIONS; i++) {
        reg->connections[i].targetRegionId = -1;
        reg->connections[i].distanceKM = 0.0;
    }

    // Initialize drawing configurations
    R_DRAW_UTILS dC = {{0.0, 0.0}, 0.0};
    reg->drawConf = dC;

    return reg;
}

// Adds a new connection (edge) to another region
void addRegionConnection(REGION *r, int targetId, double km) {
    if (!r || r->numConnections >= MAX_REGION_CONNECTIONS) return;
    r->connections[r->numConnections].targetRegionId = targetId;
    r->connections[r->numConnections].distanceKM = km;
    r->numConnections++;
}

// ------------------
// For Hash Functions
// Creates and initializes the Region Hash Table
REGION_HASH_TABLE* createRegionHashTable() {
    REGION_HASH_TABLE *ht = (REGION_HASH_TABLE*)calloc(1, sizeof(REGION_HASH_TABLE));
    return ht;
}

// Inserts a Region structure into the Hash Table using the region's ID as the key
void insertRegionInHash(REGION_HASH_TABLE *ht, const REGION *region) {
    if (!ht || !region) return;

    unsigned int index = hashFunction(region->id, REGION_HASH_TABLE_SIZE);
    REGION_NODE *new_node = (REGION_NODE*)malloc(sizeof(REGION_NODE));
    if (!new_node) return;

    new_node->data = *region;
    new_node->next = ht->table[index];
    ht->table[index] = new_node;
    ht->count++;
}

// Searches for a region by its ID in the Hash Table
REGION* searchRegionInHash(REGION_HASH_TABLE *ht, int region_id) {
    if (!ht) return NULL;

    unsigned int index = hashFunction(region_id, REGION_HASH_TABLE_SIZE);
    REGION_NODE *current = ht->table[index];

    while (current != NULL) {
        if (current->data.id == region_id) {
            return &current->data;
        }
        current = current->next;
    }
    return NULL;
}

// Frees all memory allocated for the Hash Table and its nodes
void freeRegionInHash(REGION_HASH_TABLE *ht) {
    if (!ht) return;
    for (int i = 0; i < REGION_HASH_TABLE_SIZE; i++) {
        RegionsCount = 0;
        REGION_NODE *current = ht->table[i];
        while (current != NULL) {
            REGION_NODE *temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(ht);
}
