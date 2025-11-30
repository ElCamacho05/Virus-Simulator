// DAO_General.h

#ifndef DAO_GENERAL_H
#define DAO_GENERAL_H

#include "Clases/Person.h" // trae PERSON y PERSON_HASH_TABLE
#include "Clases/Virus.h"  // trae STRAIN y STRAIN_HASH_TABLE
#include "Clases/Regions.h"// trae REGION y REGION_HASH_TABLE

// Global structure for program's data
typedef struct BioSimData{
    // Tablas Hash para guardar los datos (acceso O(1) por id)
    PERSON_HASH_TABLE  *persons_table;    
    STRAIN_HASH_TABLE  *cepas_hash_table; 
    REGION_HASH_TABLE  *regions_table; 

    // Límites (opcionales, para validaciones y estadísticas)
    int max_individuos;
    int max_territorios;

} BIO_SIM_DATA;

// Prototipos que el DAO usará
BIO_SIM_DATA* createBiosimData(int max_i, int max_t);
void free_biosim_data(BIO_SIM_DATA *data);

// Main function for loading the initial data (archivos CSV)
BIO_SIM_DATA* load_initial_data(const char *cepas_f, const char *terr_f, const char *ind_f, const char *cont_f);

// Consultas O(1) usando las tablas hash
STRAIN* get_cepa_by_id(BIO_SIM_DATA *data, int id);
PERSON* get_person_by_id(BIO_SIM_DATA *data, int id);
REGION* get_region_by_id(BIO_SIM_DATA *data, int id);

// Función de control de historial (Punto 8)
void save_contagion_history(BIO_SIM_DATA *data, int dia_simulacion);



#endif