// Clases/Virus.h

#ifndef VIRUS_H
#define VIRUS_H

#include <stdbool.h>

// VIRUS constants
#define ALPHABET_SIZE 27
#define MAX_VIRUSES 100 

// --- HASH TABLE CONSTANTS (FOR O(1) ACCESS) ---
#define HASH_TABLE_SIZE 101 // Tamaño primo para el hashing

// VIRUS structures (Renombrada a CEPA para claridad del proyecto)
typedef struct Cepa{
    int id;
    char name[20];
    double beta; 
    double caseFatalityRatio; // Letalidad
    double recovery; // Gamma de recuperación
} CEPA;


// --- ESTRUCTURAS DE HASH TABLE (O(1) DAO) ---
typedef struct CepaNode {
    CEPA data;
    struct CepaNode *next;
} CepaNode;

typedef struct {
    CepaNode *table[HASH_TABLE_SIZE];
    int count;
} CepaHashTable;


// --- ESTRUCTURAS DE TRIE (Para Tarea 7: Agrupación O(L)) ---
struct TrieNode {
    struct TrieNode *children[26];
    bool isEndEfWord;
    CEPA *cepa; // Lo cambiamos de VIRUS* a CEPA*
};


// VIRUSES variables
int virusesCount = 0;

// Prototipos de HASH TABLE (Implementados en Virus.c)
CepaHashTable* create_cepa_hash_table();
void hash_table_insert_cepa(CepaHashTable *ht, const CEPA *cepa);
CEPA* hash_table_lookup_cepa(CepaHashTable *ht, int cepa_id);
void free_cepa_hash_table(CepaHashTable *ht);


// VIRUS functions (Prototypes for Trie, already existing)
bool isempty(struct TrieNode *root);
// ... (Otros prototipos de Trie) ...

#endif