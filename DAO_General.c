// DAO_General.c

#include "DAO_General.h"
#include "Clases/Virus.h" // funciones para cepas
#include "Clases/Person.h" // funciones para personas
#include "Clases/Regions.h"// funciones para regiones
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Crea la estructura principal y las tablas hash internas
BIO_SIM_DATA* createBiosimData(int max_i, int max_t) {
    BIO_SIM_DATA *data = (BIO_SIM_DATA*)malloc(sizeof(BIO_SIM_DATA));
    if (!data) return NULL;
    data->max_individuos = max_i;
    data->max_territorios = max_t;

    // Crear las tablas hash usando las funciones definidas en cada módulo
    data->cepas_hash_table = createStrainHashTable();
    data->persons_table = createPersonHashTable();
    data->regions_table = createRegionHashTable();

    if (!data->cepas_hash_table || !data->persons_table || !data->regions_table) {
        // Liberar lo creado si algo falla
        if (data->cepas_hash_table) freeStrainInHash(data->cepas_hash_table);
        if (data->persons_table) freePersonInHash(data->persons_table);
        if (data->regions_table) freeRegionInHash(data->regions_table);
        free(data);
        return NULL;
    }

    return data;
}


// Carga inicial de archivos CSV en las tablas hash correspondientes
BIO_SIM_DATA* load_initial_data(const char *cepas_f, const char *terr_f, const char *ind_f, const char *cont_f) {
    // Crear con límites razonables; se usan para estadísticas, no arrays
    const int DEFAULT_MAX_I = 10000;
    const int DEFAULT_MAX_T = 1000;
    BIO_SIM_DATA *data = createBiosimData(DEFAULT_MAX_I, DEFAULT_MAX_T);
    if (!data) return NULL;

    FILE *fp;

    // --- Carga de Cepas ---
    fp = fopen(cepas_f, "r");
    if (fp) {
        char line[256];
        printf("Cargando cepas desde '%s'...\n", cepas_f);
        int cepa_count = 0;
        while (fgets(line, sizeof(line), fp)) {
            STRAIN s = {0};
            // Formato esperado: id,name,beta,caseFatalityRatio,recovery
            if (sscanf(line, "%d,%19[^,],%lf,%lf,%lf", &s.id, s.name, &s.beta, &s.caseFatalityRatio, &s.recovery) >= 1) {
                insertStrainInHash(data->cepas_hash_table, &s);
                cepa_count++;
            }
        }
        fclose(fp);
        printf("/ Se cargaron %d cepas.\n", cepa_count);
    } else {
        fprintf(stderr, "!!!Aviso: No se encontró archivo de cepas '%s'. Continuando...\n", cepas_f);
    }

    // --- Carga de Regiones ---
    fp = fopen(terr_f, "r");
    if (fp) {
        char line[256];
        printf("Cargando regiones desde '%s'...\n", terr_f);
        int region_count = 0;
        while (fgets(line, sizeof(line), fp)) {
            REGION r = {0};
            char namebuf[64] = {0};
            if (sscanf(line, "%d,%63[^,]", &r.id, namebuf) >= 1) {
                strncpy(r.name, namebuf, sizeof(r.name)-1);
                r.peopleIDs = NULL;
                r.populationCount = 0;
                r.infected = 0;
                insertRegionInHash(data->regions_table, &r);
                region_count++;
            }
        }
        fclose(fp);
        printf("/ Se cargaron %d regiones.\n", region_count);
    } else {
        fprintf(stderr, "!!!Aviso: No se encontró archivo de regiones '%s'. Continuando...\n", terr_f);
    }

    // --- Carga de Individuos ---
    fp = fopen(ind_f, "r");
    if (fp) {
        char line[512];
        printf("Cargando individuos desde '%s'...\n", ind_f);
        int person_count = 0;
        while (fgets(line, sizeof(line), fp)) {
            PERSON p = {0};
            char namebuf[128] = {0};
            int status_int = 0;
            // Esperado: id,name,regionID,initialDegree,initialRisk,status,actualStrainID,daysInfected
            int scanned = sscanf(line, "%d,%127[^,],%d,%lf,%lf,%d,%d,%d", &p.id, namebuf, &p.regionID, &p.initialDegree, &p.initialRisk, &status_int, &p.actualStrainID, &p.daysInfected);
            if (scanned >= 3) {
                strncpy(p.name, namebuf, sizeof(p.name)-1);
                if (scanned < 6) status_int = HEALTH;
                p.status = (HealthStatus)status_int;
                insertPersonInHash(data->persons_table, &p);
                person_count++;
            }
        }
        fclose(fp);
        printf("/ Se cargaron %d individuos.\n", person_count);
    } else {
        fprintf(stderr, "!!!Aviso: No se encontró archivo de individuos '%s'. Continuando...\n", ind_f);
    }

    // --- Carga de Contactos (opcional) ---
    // Dependiendo del formato de contactos, implementar acá la lógica para enlazar personas o generar grafos.
    if (cont_f) {
        // Placeholder: abrir y procesar contactos si existe
        FILE *fc = fopen(cont_f, "r");
        if (fc) {
            // Implementación específica pendiente
            fclose(fc);
        }
    }

    printf("\n========== RESUMEN CARGA DE DATOS ==========\n");
    printf("Cepas cargadas:     %d\n", data->cepas_hash_table->count);
    printf("Regiones cargadas:  %d\n", data->regions_table->count);
    printf("Individuos cargados: %d\n", data->persons_table->count);
    printf("==========================================\n\n");

    return data;
}


// Consultas O(1)
STRAIN* get_cepa_by_id(BIO_SIM_DATA *data, int id) {
    if (!data || !data->cepas_hash_table) return NULL;
    return searchStrainInHash(data->cepas_hash_table, id);
}

PERSON* get_person_by_id(BIO_SIM_DATA *data, int id) {
    if (!data || !data->persons_table) return NULL;
    return searchPersonInHash(data->persons_table, id);
}

REGION* get_region_by_id(BIO_SIM_DATA *data, int id) {
    if (!data || !data->regions_table) return NULL;
    return searchRegionInHash(data->regions_table, id);
}

// Liberación de memoria
void free_biosim_data(BIO_SIM_DATA *data) {
    if (!data) return;
    if (data->cepas_hash_table) freeStrainInHash(data->cepas_hash_table);
    if (data->persons_table) freePersonInHash(data->persons_table);
    if (data->regions_table) freeRegionInHash(data->regions_table);
    free(data);
}


// Placeholder: guardar historial (implementación según formato deseado)
void save_contagion_history(BIO_SIM_DATA *data, int dia_simulacion) {
    (void)data; (void)dia_simulacion; // evitar warnings por parámetros no usados
    // Implementar serialización de historial si se requiere
}