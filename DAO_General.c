// DAO_General.c

#include "DAO_General.h"
#include "Clases/Virus.h"
#include "Clases/Person.h"
#include "Clases/Regions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Initialize main data structure with hash tables for strains, persons, and regions
BIO_SIM_DATA* createBiosimData(int max_i, int max_t) {
    BIO_SIM_DATA *data = (BIO_SIM_DATA*)malloc(sizeof(BIO_SIM_DATA));
    if (!data) return NULL;
    data->max_individuos = max_i;
    data->max_territorios = max_t;

    data->cepas_hash_table = createStrainHashTable();
    data->persons_table = createPersonHashTable();
    data->regions_table = createRegionHashTable();

    if (!data->cepas_hash_table || !data->persons_table || !data->regions_table) {
        if (data->cepas_hash_table) freeStrainInHash(data->cepas_hash_table);
        if (data->persons_table) freePersonInHash(data->persons_table);
        if (data->regions_table) freeRegionInHash(data->regions_table);
        free(data);
        return NULL;
    }

    return data;
}


// Load CSV files into hash tables for strains, regions, and persons
BIO_SIM_DATA* load_initial_data(const char *cepas_f, const char *terr_f, const char *ind_f, const char *cont_f) {
    const int DEFAULT_MAX_I = 10000;
    const int DEFAULT_MAX_T = 1000;
    BIO_SIM_DATA *data = createBiosimData(DEFAULT_MAX_I, DEFAULT_MAX_T);
    if (!data) return NULL;

    FILE *fp;

    // Load strains: id,name,beta,caseFatalityRatio,recovery
    fp = fopen(cepas_f, "r");
    if (fp) {
        char line[256];
        printf("Loading strains from '%s'...\n", cepas_f);
        int cepa_count = 0;
        while (fgets(line, sizeof(line), fp)) {
            STRAIN s = {0};
            if (sscanf(line, "%d,%19[^,],%lf,%lf,%lf", &s.id, s.name, &s.beta, &s.caseFatalityRatio, &s.recovery) >= 1) {
                insertStrainInHash(data->cepas_hash_table, &s);
                cepa_count++;
            }
        }
        fclose(fp);
        printf("  ✓ Loaded %d strains.\n", cepa_count);
    } else {
        fprintf(stderr, "  ⚠ Warning: Strains file '%s' not found. Continuing...\n", cepas_f);
    }

    // Load regions: id,name
    fp = fopen(terr_f, "r");
    if (fp) {
        char line[256];
        printf("Loading regions from '%s'...\n", terr_f);
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
        printf("  ✓ Loaded %d regions.\n", region_count);
    } else {
        fprintf(stderr, "  ⚠ Warning: Regions file '%s' not found. Continuing...\n", terr_f);
    }

    // Load persons: id,name,regionID,initialDegree,initialRisk,status,actualStrainID,daysInfected
    fp = fopen(ind_f, "r");
    if (fp) {
        char line[512];
        printf("Loading persons from '%s'...\n", ind_f);
        int person_count = 0;
        while (fgets(line, sizeof(line), fp)) {
            PERSON p = {0};
            char namebuf[128] = {0};
            int status_int = 0;
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
        printf("  ✓ Loaded %d persons.\n", person_count);
    } else {
        fprintf(stderr, "  ⚠ Warning: Persons file '%s' not found. Continuing...\n", ind_f);
    }

    // TODO: Load contacts (person-to-person graph)
    if (cont_f) {
        FILE *fc = fopen(cont_f, "r");
        if (fc) {
            fclose(fc);
        }
    }

    printf("\n========== DATA LOAD SUMMARY ==========\n");
    printf("Strains loaded:     %d\n", data->cepas_hash_table->count);
    printf("Regions loaded:     %d\n", data->regions_table->count);
    printf("Persons loaded:     %d\n", data->persons_table->count);
    printf("======================================\n\n");

    return data;
}


// O(1) lookups
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

// Free all data and hash tables
void free_biosim_data(BIO_SIM_DATA *data) {
    if (!data) return;
    if (data->cepas_hash_table) freeStrainInHash(data->cepas_hash_table);
    if (data->persons_table) freePersonInHash(data->persons_table);
    if (data->regions_table) freeRegionInHash(data->regions_table);
    free(data);
}


// TODO: Save contagion history to file
void save_contagion_history(BIO_SIM_DATA *data, int dia_simulacion) {
    (void)data; (void)dia_simulacion;
}