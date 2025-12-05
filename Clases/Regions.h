#ifndef REGIONS_H
#define REGIONS_H

// REGION constants
#define MAX_REGIONS 30 
#define REGION_HASH_TABLE_SIZE 41 // prime number for initializing hash table
#define MAX_REGION_CONNECTIONS 10

// REGION variables
extern int RegionsCount;

// REGION structures
// Defines a connection (edge) between two regions
typedef struct {
    int targetRegionId;
    double distanceKM;
} RegionConnection;

// Drawing utilities for graphical representation
typedef struct RegionDrawUtils {
    double pos[2];
    double radio;
} R_DRAW_UTILS;

// Main structure representing a geographical region
typedef struct Region{
    int id;
    char name[20];
    int *peopleIDs;

    int populationCount;    
    int infectedCount;
    int deathCount;

    // Connections to other regions, forming a graph
    RegionConnection connections[MAX_REGION_CONNECTIONS];
    int numConnections;

    R_DRAW_UTILS drawConf;
} REGION;

// Node structure for the Region Hash Table (linked list for collisions)
typedef struct RegionNode { 
    REGION data;
    struct RegionNode *next;
} REGION_NODE;

// Hash Table structure for efficient Region lookup
typedef struct { 
    REGION_NODE *table[REGION_HASH_TABLE_SIZE];
    int count;
} REGION_HASH_TABLE;


/*
---------------
REGION Functions
---------------
*/

// ---------------
// Basic Functions
REGION *createRegion(int id, char name[]);
void addRegionConnection(REGION *r, int targetId, double km);

// ------------------
// For Hash Functions
REGION_HASH_TABLE* createRegionHashTable();
void insertRegionInHash(REGION_HASH_TABLE *ht, const REGION *region);
REGION* searchRegionInHash(REGION_HASH_TABLE *ht, int region_id);
void freeRegionInHash(REGION_HASH_TABLE *ht);

#endif