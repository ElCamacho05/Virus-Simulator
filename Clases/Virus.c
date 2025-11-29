// Clases/Virus.c (
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Self Library
#include "Person.h"
#include "Virus.h"


// ------------------------------------------------------------------
// --- IMPLEMENTACIÓN DE HASH TABLE (PARA DAO O(1)) ---
// ------------------------------------------------------------------

// Función Hash simple: Mapea la clave (cepa_id) a un índice de la tabla.
unsigned int hash_function(int key) {
    return (unsigned int)key % HASH_TABLE_SIZE;
}

CepaHashTable* create_cepa_hash_table() {
    CepaHashTable *ht = (CepaHashTable*)calloc(1, sizeof(CepaHashTable));
    return ht;
}

// Inserta una Cepa en la Tabla Hash (O(1) promedio)
void hash_table_insert_cepa(CepaHashTable *ht, const CEPA *cepa) {
    if (!ht || !cepa) return;

    unsigned int index = hash_function(cepa->id); // Usamos el ID como clave
    CepaNode *new_node = (CepaNode*)malloc(sizeof(CepaNode));
    if (!new_node) return;

    new_node->data = *cepa;
    new_node->next = ht->table[index];
    ht->table[index] = new_node;
    ht->count++;
}

// Busca una Cepa por ID (O(1) promedio)
CEPA* hash_table_lookup_cepa(CepaHashTable *ht, int cepa_id) {
    if (!ht) return NULL;

    unsigned int index = hash_function(cepa_id);
    CepaNode *current = ht->table[index];

    while (current != NULL) {
        if (current->data.id == cepa_id) {
            return &current->data;
        }
        current = current->next;
    }
    return NULL;
}

void free_cepa_hash_table(CepaHashTable *ht) {
    if (!ht) return;
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        CepaNode *current = ht->table[i];
        while (current != NULL) {
            CepaNode *temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(ht);
}
