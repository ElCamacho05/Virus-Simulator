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
REGION *createRegion(int id, char name[]) {
    REGION *reg = (REGION *) malloc(sizeof(REGION));
    if (reg == NULL) return NULL;
    
    reg->id = id;
    strcpy(reg->name, name);
    reg->infected = 0;
    
    reg->populationCount = 0;
    reg->peopleIDs = (int *) calloc(MAX_POPULATION, sizeof(int)); 

    R_DRAW_UTILS dC = {{0.0, 0.0}, 0.0};
    reg->drawConf = dC;

    return reg;
}

// ------------------
// For Hash Functions
REGION_HASH_TABLE* createRegionHashTable() {
    REGION_HASH_TABLE *ht = (REGION_HASH_TABLE*)calloc(1, sizeof(REGION_HASH_TABLE));
    return ht;
}

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
