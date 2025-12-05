// DAO_General.h

#ifndef DAO_GENERAL_H
#define DAO_GENERAL_H

#include "Clases/Person.h"
#include "Clases/Virus.h"
#include "Clases/Regions.h"
#include "Algorithms/Algorithms.h"

// --- STRUCTURES FOR HISTORY ---
// Represents a single day's state for a person
typedef struct DailyHistory {
    int day;
    int status; // HealthStatus
    int strain_id;
} DAILY_HISTORY_ENTRY;

// Structure that stores the history for a single patient
typedef struct PersonHistory {
    DAILY_HISTORY_ENTRY *entries;
    int entry_count;
    int capacity;
} PERSON_HISTORY;

// Hash Table History Node (Indexed by Person ID)
typedef struct HistoryNode {
    int person_id;
    PERSON_HISTORY history;
    struct HistoryNode *next;
} HISTORY_NODE;

// Centralized History Hash Table
typedef struct {
    HISTORY_NODE *table[PERSON_HASH_TABLE_SIZE]; // Reuses the Person table size
    int count;
} HISTORY_HASH_TABLE;

// Global structure for program's data
typedef struct BioSimData {
    PERSON_HASH_TABLE  *persons_table;    
    STRAIN_HASH_TABLE  *cepas_hash_table; 
    REGION_HASH_TABLE  *regions_table;

    // Arrays to keep data sorted for quick access (MergeSort pre-processing)
    // Daily sorting occurs in 'void idle'
    PERSON **personArray;
    REGION *regionArray;
    STRAIN *strainArray;

    // Hash Table for history (O(1) lookup per patient)
    HISTORY_HASH_TABLE *history_table;

    // Event Queue for simulation
    MinHeap *eventQueue; 

    // Optimized infections array for O(Infected) propagation
    int *activeInfectedIDs; 
    int infectedCount;
    int deathCount;
    int isolatedCount;

    int activeStrainCount;
    
    double totalRisk;

    int max_individuos;
    int max_territorios;
} BIO_SIM_DATA;

// DAO public interface
BIO_SIM_DATA* createBiosimData(int max_i, int max_t);
void free_biosim_data(BIO_SIM_DATA *data);

// Load CSV files into data structure
BIO_SIM_DATA* load_initial_data(const char *cepas_f, const char *terr_f, const char *ind_f, const char *cont_f, const char *reg_conn_f);

// O(1) lookups on hash tables
STRAIN* get_cepa_by_id(BIO_SIM_DATA *data, int id);
PERSON* get_person_by_id(BIO_SIM_DATA *data, int id);
REGION* get_region_by_id(BIO_SIM_DATA *data, int id);

// Initialize drawing positions for regions and persons
void initializePositions(BIO_SIM_DATA *data);

// Save simulation state
void save_contagion_history(BIO_SIM_DATA *data, int dia_simulacion);

// Function to initialize history hash table
HISTORY_HASH_TABLE* createHistoryHashTable();

// Function for O(1) history lookup by person ID and day
DAILY_HISTORY_ENTRY* get_history_by_id_and_day(BIO_SIM_DATA *data, int person_id, int day);
#endif