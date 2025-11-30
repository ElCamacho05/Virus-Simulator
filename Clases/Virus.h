#ifndef VIRUS_H
#define VIRUS_H


// STRAIN constants
#define ALPHABET_SIZE 27
#define MAX_VIRUSES 100 
#define VIRUS_HASH_TABLE_SIZE 131 // prime number for initializing hash table


// STRAIN Variables
extern int virusesCount;


/*
-----------------
STRAIN structures
-----------------
*/

typedef struct Strain{ // MAIN structure
    int id;
    char name[20];
    double beta; 
    double caseFatalityRatio;
    double recovery;
} STRAIN;

typedef struct StrainNode { // HASH wrapper structure
    STRAIN data;
    struct StrainNode *next;
} STRAIN_NODE;

typedef struct { // HASH TABLE centralized structure
    STRAIN_NODE *table[VIRUS_HASH_TABLE_SIZE];
    int count;
} STRAIN_HASH_TABLE;

struct TrieNode { // TRIE for strain classification
    struct TrieNode *children[26];
    int isEndEfWord;
    STRAIN *strain;
};


/*
---------------
STRAIN Functions
---------------
*/

// ------------------
// For Hash Functions
STRAIN_HASH_TABLE* createStrainHashTable();
void insertStrainInHash(STRAIN_HASH_TABLE *ht, const STRAIN *strain);
STRAIN* searchStrainInHash(STRAIN_HASH_TABLE *ht, int strain_id);
void freeStrainInHash(STRAIN_HASH_TABLE *ht);

//------------------------------------
// For Strain Logic and Classification
int isempty(struct TrieNode *root);
void insert(struct TrieNode *root, const char *key);
struct TrieNode *createNode();
struct TrieNode* search(struct TrieNode *root, const char *key);
struct TrieNode *deletehelper(struct TrieNode *root, const char *key, int depth);
void deletekey(struct TrieNode *root, const char *key);

#endif