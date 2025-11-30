// DAO_General.c

#include "DAO_General.h"
#include "Clases/Virus.h"
#include "Clases/Person.h"
#include "Clases/Regions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
        printf(" / Loading strains from '%s'...\n", cepas_f);
        int cepa_count = 0;
        while (fgets(line, sizeof(line), fp)) {
            STRAIN s = {0};
            if (sscanf(line, "%d,%19[^,],%lf,%lf,%lf", &s.id, s.name, &s.beta, &s.caseFatalityRatio, &s.recovery) >= 1) {
                insertStrainInHash(data->cepas_hash_table, &s);
                cepa_count++;
            }
        }
        fclose(fp);
        printf(" / Loaded %d strains.\n", cepa_count);
    } else {
        fprintf(stderr, "!!!Warning: Strains file '%s' not found. Continuing...\n", cepas_f);
    }

    // Load regions: id,name
    fp = fopen(terr_f, "r");
    if (fp) {
        char line[256];
        printf(" / Loading regions from '%s'...\n", terr_f);
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
        printf(" / Loaded %d regions.\n", region_count);
    } else {
        fprintf(stderr, "!!!Warning: Regions file '%s' not found. Continuing...\n", terr_f);
    }

    // Load persons: id,name,regionID,initialDegree,initialRisk,status,actualStrainID,daysInfected
    fp = fopen(ind_f, "r");
    if (fp) {
        char line[512];
        printf(" / Loading persons from '%s'...\n", ind_f);
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
                p.infectedBy = -1;
                P_DRAW_UTILS drawConf = {{0.0, 0.0}};
                p.drawConf = drawConf;
                insertPersonInHash(data->persons_table, &p);
                person_count++;
            }
        }
        fclose(fp);
        printf(" / Loaded %d persons.\n", person_count);
    } else {
        fprintf(stderr, "!!!Warning: Persons file '%s' not found. Continuing...\n", ind_f);
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

    /* Initialize drawing positions for regions and persons */
    initializePositions(data);

    return data;
}

/*
 * Initialize radial positions for regions in a circle pattern (sunflower/girasol arrangement)
 * Positions are calculated based on region count and angle increment of 360/count degrees
 */
void initializePositions(BIO_SIM_DATA *data) {
    if (!data || !data->regions_table || !data->persons_table) {
        fprintf(stderr, "Error: Invalid data structure in initializePositions\n");
        return;
    }

    REGION_HASH_TABLE *regions_table = data->regions_table;
    PERSON_HASH_TABLE *persons_table = data->persons_table;
    int num_regions = regions_table->count;

    if (num_regions == 0) {
        fprintf(stderr, "Warning: No regions to initialize positions for\n");
        return;
    }

    /* ===== STEP 1: Initialize radial positions for regions ===== */
    printf("\n[Initializing] Region positions (radial arrangement)...\n");
    
    double angle_increment = 2.0 * 3.14159265359 / num_regions;  /* 360 degrees / num_regions */
    double region_distance = 200.0;  /* Distance from center (0,0) */
    double region_radius = 30.0;     /* Radius of each region circle */
    int region_idx = 0;

    /* Iterate through hash table to assign positions to regions */
    for (int i = 0; i < REGION_HASH_TABLE_SIZE; i++) {
        REGION_NODE *node = regions_table->table[i];
        while (node != NULL) {
            double angle = region_idx * angle_increment;
            double x = region_distance * cos(angle);
            double y = region_distance * sin(angle);
            
            node->data.drawConf.pos[0] = x;
            node->data.drawConf.pos[1] = y;
            node->data.drawConf.radio = region_radius;
            
            printf("  Region %d (%s): pos=(%.2f, %.2f), radio=%.2f\n", 
                   node->data.id, node->data.name, x, y, region_radius);
            
            region_idx++;
            node = node->next;
        }
    }

    /* ===== STEP 2: Counting Sort - Sort persons by region ID (O(N)) ===== */
    printf("\n[Initializing] Sorting persons by region ID (counting sort)...\n");
    
    int total_persons = persons_table->count;
    if (total_persons == 0) {
        fprintf(stderr, "Warning: No persons to initialize positions for\n");
        return;
    }

    /* Allocate temporary array for sorted persons */
    PERSON *sorted_persons = (PERSON*)malloc(total_persons * sizeof(PERSON));
    if (!sorted_persons) {
        fprintf(stderr, "Error: Memory allocation failed for sorted_persons\n");
        return;
    }

    /* Allocate count array for each region ID */
    int max_region_id = 0;
    PERSON_NODE *tmp_node;
    for (int i = 0; i < PERSON_HASH_TABLE_SIZE; i++) {
        tmp_node = persons_table->table[i];
        while (tmp_node != NULL) {
            if (tmp_node->data.regionID > max_region_id) {
                max_region_id = tmp_node->data.regionID;
            }
            tmp_node = tmp_node->next;
        }
    }

    int *count = (int*)calloc(max_region_id + 1, sizeof(int));
    if (!count) {
        fprintf(stderr, "Error: Memory allocation failed for count array\n");
        free(sorted_persons);
        return;
    }

    /* Step 2a: Count persons per region */
    for (int i = 0; i < PERSON_HASH_TABLE_SIZE; i++) {
        tmp_node = persons_table->table[i];
        while (tmp_node != NULL) {
            count[tmp_node->data.regionID]++;
            tmp_node = tmp_node->next;
        }
    }

    /* Step 2b: Convert counts to starting indices */
    int total = 0;
    for (int i = 0; i <= max_region_id; i++) {
        int temp = count[i];
        count[i] = total;
        total += temp;
    }

    /* Step 2c: Place persons in sorted order */
    for (int i = 0; i < PERSON_HASH_TABLE_SIZE; i++) {
        tmp_node = persons_table->table[i];
        while (tmp_node != NULL) {
            int region_id = tmp_node->data.regionID;
            int pos = count[region_id]++;
            sorted_persons[pos] = tmp_node->data;
            tmp_node = tmp_node->next;
        }
    }

    /* ===== STEP 3: Initialize person positions within region circles ===== */
    printf("[Initializing] Person positions (within region radii)...\n");
    
    int current_region_id = -1;
    REGION *current_region = NULL;
    double person_radius = 0.0;
    int persons_in_current_region = 0;

    for (int i = 0; i < total_persons; i++) {
        PERSON *person = &sorted_persons[i];

        /* Get region info when moving to a new region */
        if (person->regionID != current_region_id) {
            current_region_id = person->regionID;
            current_region = searchRegionInHash(regions_table, current_region_id);
            person_radius = current_region ? current_region->drawConf.radio : 30.0;
            persons_in_current_region = 0;
        }

        /* Calculate random position within region radius */
        double r = ((double)rand() / RAND_MAX) * person_radius;  /* Random radius within region */
        double theta = ((double)rand() / RAND_MAX) * 2.0 * 3.14159265359;  /* Random angle */

        if (current_region) {
            person->drawConf.pos[0] = current_region->drawConf.pos[0] + r * cos(theta);
            person->drawConf.pos[1] = current_region->drawConf.pos[1] + r * sin(theta);
        } else {
            person->drawConf.pos[0] = r * cos(theta);
            person->drawConf.pos[1] = r * sin(theta);
        }

        persons_in_current_region++;

        if (persons_in_current_region <= 3 || i == total_persons - 1) {  /* Show first 3 and last */
            printf("  Person %d (Region %d): pos=(%.2f, %.2f)\n", 
                   person->id, person->regionID, 
                   person->drawConf.pos[0], person->drawConf.pos[1]);
        }
    }

    /* ===== STEP 4: Update hash table with sorted positions ===== */
    /* Reconstruct hash table entries with updated position data */
    for (int i = 0; i < total_persons; i++) {
        PERSON *person = &sorted_persons[i];
        PERSON *hash_person = searchPersonInHash(persons_table, person->id);
        if (hash_person) {
            hash_person->drawConf = person->drawConf;
        }
    }

    /* Cleanup */
    free(sorted_persons);
    free(count);

    printf("[Success] Position initialization completed\n\n");
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