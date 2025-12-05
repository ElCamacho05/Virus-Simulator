// Self Library
#include "Virus.h"

// General Libraries
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

// Other Classes libraries
#include "../Algorithms/Algorithms.h"
// STRAIN Variables
int virusesCount = 0;

/*
---------------
STRAIN Functions
---------------
*/

// ------------------
// Basic Functions

// ------------------
// For Hash Functions

// Creates and initializes the Strain Hash Table
STRAIN_HASH_TABLE* createStrainHashTable() {
    STRAIN_HASH_TABLE *ht = (STRAIN_HASH_TABLE*)calloc(1, sizeof(STRAIN_HASH_TABLE));
    return ht;
}

// Inserts a Strain structure into the Hash Table using the strain's ID as the key
void insertStrainInHash(STRAIN_HASH_TABLE *ht, const STRAIN *strain) {
    if (!ht || !strain) return;

    unsigned int index = hashFunction(strain->id, VIRUS_HASH_TABLE_SIZE);
    STRAIN_NODE *new_node = (STRAIN_NODE*)malloc(sizeof(STRAIN_NODE));
    if (!new_node) return;

    new_node->data = *strain;
    new_node->next = ht->table[index];
    ht->table[index] = new_node;
    ht->count++;
}

// Searches for a strain by its ID in the Hash Table
STRAIN* searchStrainInHash(STRAIN_HASH_TABLE *ht, int strain_id) {
    if (!ht) return NULL;

    unsigned int index = hashFunction(strain_id, VIRUS_HASH_TABLE_SIZE);
    STRAIN_NODE *current = ht->table[index];

    while (current != NULL) {
        if (current->data.id == strain_id) {
            return &current->data;
        }
        current = current->next;
    }
    return NULL;
}

// Frees all memory allocated for the Hash Table and its nodes
void freeStrainInHash(STRAIN_HASH_TABLE *ht) {
    if (!ht) return;
    for (int i = 0; i < VIRUS_HASH_TABLE_SIZE; i++) {
        STRAIN_NODE *current = ht->table[i];
        while (current != NULL) {
            STRAIN_NODE *temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(ht);
}

//------------------------------------
// For Strain Logic and Classification
// Checks if a Trie node has no children
int isempty(struct TrieNode *root) {
    for (int i = 0; i < 26; i++) {
        if (root->children[i] != NULL) {
            return 0;
        }
    }
    return 1;
}

// Inserts a strain name (key) into the Trie structure
void insert(struct TrieNode *root, const char *key) {
    if (virusesCount >= 50) return; 
    struct TrieNode *current = root;
    for (int i = 0; i < strlen(key); i++) {
        char c = key[i];
        int index =-1;

        // Logic to calculate the index (0-25 for a-z)
        if (c >= 'a' && c <= 'z') {
            index = c - 'a';
        } else if (c >= 'A' && c <= 'Z') {
            index = c - 'A'; // Convert uppercase to index 0-25
        } else if (c == '-') {
            continue; // Ignore hyphens
        } else {
            continue; // Ignore any other invalid character
        }

        // Extra safety validation
        if (index < 0 || index >= 26) continue;

        if (current->children[index] == NULL) {
            current->children[index] = createNode();
        }
        current = current->children[index];
    }
    current->isEndEfWord = 1;
    virusesCount++;
}

// Creates a new, empty Trie node
struct TrieNode *createNode() {
    struct TrieNode *node = (struct TrieNode *)malloc(sizeof(struct TrieNode));
    node->isEndEfWord = 0;
    for (int i = 0; i < 26; i++) {
        node->children[i] = NULL;
    }
    return node;
}

// Searches for a key (strain name) in the Trie
struct TrieNode* search(struct TrieNode *root, const char *key) {
    struct TrieNode *current = root;
    for (int i = 0; i < strlen(key); i++) {
        int index = key[i] - 'a';
        if (current->children[index] == NULL) {
            return NULL;
        }
        current = current->children[index];
    }
    if (current != NULL && current->isEndEfWord)
        return current;
    return NULL;
}

// Recursive helper function for key deletion
struct TrieNode *deletehelper(struct TrieNode *root, const char *key, int depth) {
    if (root == NULL) {
        return NULL;
    }
    if (depth == strlen(key)) {
        if (root->isEndEfWord) {
            root->isEndEfWord = 0;
        }
        if (isempty(root)) {
            free(root);
            root = NULL;
        }
        return root;
    }
    int index = key[depth] - 'a';
    root->children[index] = deletehelper(root->children[index], key, depth + 1);
    if (isempty(root) && !root->isEndEfWord) {
        free(root);
        root = NULL;
    }
    return root;
}

// Deletes a key (strain name) from the Trie
void deletekey(struct TrieNode *root, const char *key) {
    deletehelper(root, key, 0);
}


// Creates a new strain by mutating an existing parent strain
STRAIN* mutate_strain(STRAIN *parent, int new_id) {
    STRAIN *new_s = (STRAIN*)malloc(sizeof(STRAIN));
    if (!new_s) return NULL;

    new_s->id = new_id;

    // NAME GENERATION: If parent is "A", child will be "A" + random letter (e.g., "AB")
    // This complies with the structure required for the Trie
    char suffix = 'A' + (rand() % 26);
    snprintf(new_s->name, 21, "%s%c", parent->name, suffix);
    // MUTATE STATISTICS (Random variation of +/- 20%)
    double variation = 0.9 + ((rand() % 40) / 100.0); 
    
    new_s->beta = parent->beta * variation;
    new_s->caseFatalityRatio = parent->caseFatalityRatio * variation;
    new_s->recovery = parent->recovery; // Recovery time is usually similar
    new_s->mutationProb = parent->mutationProb; // Inherits instability

    // Logical limits
    if (new_s->beta > 1.0) new_s->beta = 1.0;
    if (new_s->caseFatalityRatio > 1.0) new_s->caseFatalityRatio = 1.0;

    return new_s;
}