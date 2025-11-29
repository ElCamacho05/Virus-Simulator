// Clases/Person.h

#ifndef PERSON_H
#define PERSON_H

// Self Library
#include "Regions.h" 
#include "Virus.h"   // CLAVE para incluir CepaHashTable

// PERSON constants
#define MAX_POPULATION 200 

// --- ENUMS DE ESTADO (Requerido para la simulación) ---
typedef enum {
    SANO,
    INFECTADO,
    INMUNE,
    MUERTO
} EstadoSalud;

// --- PERSON STRUCTURES  ---
typedef struct Person{
    int id;
    char name[30];
    
    // MODIFICADO: Usamos ID (FK) para indexación
    int territorio_id; 
    
    double initialDegree; 
    
    // AGREGADO: Campo requerido por el PDF (en lugar de contagiousness)
    double riesgo_inicial; 
    
    // Campos de Simulación:
    EstadoSalud estado; 
    int cepa_actual_id; // ID de la cepa
    int daysInfected;

    
} PERSON;


// --- ESTRUCTURA GLOBAL DEL DAO (BioSimData) ---
typedef struct {
    // Array de Individuos (O(1) si los IDs son contiguos)
    PERSON *individuos_table;  
    
    // Tabla Hash de Cepas (Garantiza el acceso O(1))
    CepaHashTable *cepas_hash_table; 
    
    // Puntero a la estructura de territorios (definida en Regions.h)
    REGION *territorios_table; 

    int max_individuos;
    int max_territorios;

} BioSimData;

// Prototipos que el DAO usará
BioSimData* create_biosim_data(int max_i, int max_t);
void free_biosim_data(BioSimData *data);

#endif