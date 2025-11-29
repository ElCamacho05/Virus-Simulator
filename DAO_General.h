// DAO_General.h

#ifndef DAO_GENERAL_H
#define DAO_GENERAL_H

#include "Clases/Person.h" // Incluye BioSimData

// Función principal para cargar todos los datos (Punto 1).
// Se asume que el usuario crea los archivos: cepas.txt, territorios.txt, etc.
BioSimData* load_initial_data(const char *cepas_f, const char *terr_f, const char *ind_f, const char *cont_f);

// Implementación de la consulta O(1) para Cepas (Punto 8)
CEPA* get_cepa_by_id(BioSimData *data, int id);

// Función de control de historial (Punto 8)
void save_contagion_history(BioSimData *data, int dia_simulacion);

#endif