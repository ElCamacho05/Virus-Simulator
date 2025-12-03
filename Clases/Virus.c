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

STRAIN_HASH_TABLE* createStrainHashTable() {
    STRAIN_HASH_TABLE *ht = (STRAIN_HASH_TABLE*)calloc(1, sizeof(STRAIN_HASH_TABLE));
    return ht;
}

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
        char c = key[i];
        int index =-1;

        // Lógica segura para calcular el índice
        if (c >= 'a' && c <= 'z') {
            index = c - 'a';
        } else if (c >= 'A' && c <= 'Z') {
            index = c - 'A'; // Convertir mayúscula a índice 0-25
        } else if (c == '-') {
            continue; // Ignorar guiones (o podrías mapearlo a un índice especial si ampliaras el alfabeto)
        } else {
            continue; // Ignorar cualquier otro carácter no válido
        }

        // Validación extra de seguridad
        if (index < 0 || index >= 26) continue;

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


STRAIN* mutate_strain(STRAIN *parent, int new_id) {
    STRAIN *new_s = (STRAIN*)malloc(sizeof(STRAIN));
    if (!new_s) return NULL;

    new_s->id = new_id;

    // GENERAR NOMBRE: Si padre es "A", hijo será "A" + letra random (ej: "AB")
    // Esto cumple con la estructura requerida para el Trie.
    char suffix = 'A' + (rand() % 26);
    snprintf(new_s->name, 21, "%s%c", parent->name, suffix);
    // MUTAR ESTADÍSTICAS (Variación aleatoria del +/- 10%)
    double variation = 0.9 + ((rand() % 20) / 100.0); 
    
    new_s->beta = parent->beta * variation;
    new_s->caseFatalityRatio = parent->caseFatalityRatio * variation;
    new_s->recovery = parent->recovery; // La recuperación suele ser similar
    new_s->mutationProb = parent->mutationProb; // Hereda la inestabilidad

    // Limites lógicos
    if (new_s->beta > 1.0) new_s->beta = 1.0;
    if (new_s->caseFatalityRatio > 1.0) new_s->caseFatalityRatio = 1.0;

    return new_s;
}
