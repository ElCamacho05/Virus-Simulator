#ifndef REGIONS_H
#define REGIONS_H

// REGION constants
#define MAX_REGIONS 30 
#define REGION_HASH_TABLE_SIZE 41 // prime number for initializing hash table
#define MAX_REGION_CONNECTIONS 10

// REGION variables
extern int RegionsCount;

// REGION structures
typedef struct {
    int targetRegionId;
    double distanceKM;
} RegionConnection;

typedef struct RegionDrawUtils {
    double pos[2];
    double radio;
} R_DRAW_UTILS;

typedef struct Region{
    int id;
    char name[20];
    int *peopleIDs;

    int populationCount;    
    int infectedCount;
    int deathCount;

    RegionConnection connections[MAX_REGION_CONNECTIONS];
    int numConnections;

    R_DRAW_UTILS drawConf;
} REGION;

typedef struct RegionNode { // HASH wrapper structure
    REGION data;
    struct RegionNode *next;
} REGION_NODE;

typedef struct { // HASH TABLE centralized structure
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