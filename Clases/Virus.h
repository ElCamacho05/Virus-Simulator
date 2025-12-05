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

// Main structure representing a viral strain
typedef struct Strain{ 
    int id;
    char name[20];
    double beta; // Contagion rate
    double caseFatalityRatio; // Lethality rate
    double recovery; // Recovery time factor
    double mutationProb; // Probability of mutation
} STRAIN;

// Node structure for the Strain Hash Table (linked list for collisions)
typedef struct StrainNode { 
    STRAIN data;
    struct StrainNode *next;
} STRAIN_NODE;

// Hash Table structure for efficient Strain lookup
typedef struct { 
    STRAIN_NODE *table[VIRUS_HASH_TABLE_SIZE];
    int count;
} STRAIN_HASH_TABLE;

// Node structure for the TRIE (prefix tree) used for strain classification
typedef struct TrieNode { 
    struct TrieNode *children[26];
    int isEndEfWord;
    STRAIN *strain;
} TrieNode;


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
// For Strain Logic and Classification (TRIE)
int isempty(struct TrieNode *root);
void insert(struct TrieNode *root, const char *key);
struct TrieNode *createNode();
struct TrieNode* search(struct TrieNode *root, const char *key);
struct TrieNode *deletehelper(struct TrieNode *root, const char *key, int depth);
void deletekey(struct TrieNode *root, const char *key);
STRAIN* mutate_strain(STRAIN *parent, int new_id);

#endif