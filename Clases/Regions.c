// Regions.c

// General Libraries
#include <stdlib.h>
#include <string.h>

// Self Library
#include "Regions.h"

// REGION functions
REGION *createRegion(int id, char name[]) {
    REGION *reg = (REGION *) malloc(sizeof(REGION));
    if (reg == NULL) return NULL;
    
    reg->id = id;
    strcpy(reg->name, name);
    reg->infected = 0;
    
    // Inicialización de los nuevos campos
    reg->count_individuos = 0;
    // Se inicializa el array de IDs, asumiendo una población máxima inicial.
    // El tamaño debería ser MAX_POPULATION o el número M de individuos del territorio (PDF).
    reg->individuos_ids = (int *)calloc(MAX_POPULATION, sizeof(int)); 
    
    return reg;
}