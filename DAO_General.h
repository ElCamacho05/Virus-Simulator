// DAO_General.h

#ifndef DAO_GENERAL_H
#define DAO_GENERAL_H

#include "Clases/Person.h"
#include "Clases/Virus.h"
#include "Clases/Regions.h"

// Global structure for program's data
typedef struct BioSimData{
    // Hash tables for O(1) lookup by id
    PERSON_HASH_TABLE  *persons_table;    
    STRAIN_HASH_TABLE  *cepas_hash_table; 
    REGION_HASH_TABLE  *regions_table; 

    // Size limits
    int max_individuos;
    int max_territorios;

} BIO_SIM_DATA;

// DAO public interface
BIO_SIM_DATA* createBiosimData(int max_i, int max_t);
void free_biosim_data(BIO_SIM_DATA *data);

// Load CSV files into data structure
BIO_SIM_DATA* load_initial_data(const char *cepas_f, const char *terr_f, const char *ind_f, const char *cont_f);

// O(1) lookups on hash tables
STRAIN* get_cepa_by_id(BIO_SIM_DATA *data, int id);
PERSON* get_person_by_id(BIO_SIM_DATA *data, int id);
REGION* get_region_by_id(BIO_SIM_DATA *data, int id);

// Save simulation state
void save_contagion_history(BIO_SIM_DATA *data, int dia_simulacion);



#endif