// Self Library
#include "Virus.h"

// General Libraries
#include <stdlib.h>
#include <string.h>

// Other Classes libraries
#include "Person.h"
#include "Algorithms.h"

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

STRAIN_HASH_TABLE* createStrainHashTable() {
    STRAIN_HASH_TABLE *ht = (STRAIN_HASH_TABLE*)calloc(1, sizeof(STRAIN_HASH_TABLE));
    return ht;
}

void insertStrainInHash(STRAIN_HASH_TABLE *ht, const STRAIN *strain) {
    if (!ht || !strain) return;

    unsigned int index = hashFunction(strain->id);
    STRAIN_NODE *new_node = (STRAIN_NODE*)malloc(sizeof(STRAIN_NODE));
    if (!new_node) return;

    new_node->data = *strain;
    new_node->next = ht->table[index];
    ht->table[index] = new_node;
    ht->count++;
}

STRAIN* searchStrainInHash(STRAIN_HASH_TABLE *ht, int strain_id) {
    if (!ht) return NULL;

    unsigned int index = hashFunction(strain_id);
    STRAIN_NODE *current = ht->table[index];

    while (current != NULL) {
        if (current->data.id == strain_id) {
            return &current->data;
        }
        current = current->next;
    }
    return NULL;
}

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
int isempty(struct TrieNode *root) {
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (root->children[i] != NULL) {
            return 0;
        }
    }
    return 1;
}

void insert(struct TrieNode *root, const char *key) {
    if (virusesCount >= 50) return; 
    struct TrieNode *current = root;
    for (int i = 0; i < strlen(key); i++) {
        int index = key[i] - 'a';
        if (current->children[index] == NULL) {
            current->children[index] = createNode();
        }
        current = current->children[index];
    }
    current->isEndEfWord = 1;
    virusesCount++;
}

struct TrieNode *createNode() {
    struct TrieNode *node = (struct TrieNode *)malloc(sizeof(struct TrieNode));
    node->isEndEfWord = 0;
    for (int i = 0; i < 26; i++) {
        node->children[i] = NULL;
    }
    return node;
}

struct TrieNode* search(struct TrieNode *root, const char *key) {
    struct TrieNode *current = root;
    for (int i = 0; i < strlen(key); i++) {
        int index = key[i] - 'a';
        if (current->children[index] == NULL) {
            return NULL;
        }
        current = current->children[index];
    }
    // return (current != NULL && current->isEndEfWord);
    if (current != NULL && current->isEndEfWord)
        return current;
    return NULL;
}

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

void deletekey(struct TrieNode *root, const char *key) {
    deletehelper(root, key, 0);
}
