// DAO_General.c

#include "DAO_General.h"
#include "Clases/Virus.h"
#include "Clases/Person.h"
#include "Clases/Regions.h"
#include "Algorithms/Algorithms.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// --------------------------------------------------------------------------------
//                          CREACIÓN Y GESTIÓN DE MEMORIA
// --------------------------------------------------------------------------------
extern unsigned int hashFunction(int key, int size);

// --------------------------------------------------------------------------------
//                          GESTIÓN DE HISTORIAL (TAREA 8)
// --------------------------------------------------------------------------------

// Inicializa la Tabla Hash de Historial
HISTORY_HASH_TABLE* createHistoryHashTable() {
    HISTORY_HASH_TABLE *ht = (HISTORY_HASH_TABLE*)calloc(1, sizeof(HISTORY_HASH_TABLE));
    return ht;
}

// Auxiliar: Busca o crea el registro de historial de una persona (O(1) promedio)
PERSON_HISTORY* get_or_create_history(HISTORY_HASH_TABLE *ht, int person_id) {
    // Usamos el mismo tamaño de Hash que las Personas para la indexación
    unsigned int index = hashFunction(person_id, PERSON_HASH_TABLE_SIZE); 
    HISTORY_NODE *current = ht->table[index];

    // Buscar (O(1) promedio)
    while (current != NULL) {
        if (current->person_id == person_id) return &current->history;
        current = current->next;
    }

    // No existe, crear nuevo nodo e insertarlo
    HISTORY_NODE *new_node = (HISTORY_NODE*)malloc(sizeof(HISTORY_NODE));
    if (!new_node) return NULL;
    
    new_node->person_id = person_id;
    new_node->history.capacity = 50; // Capacidad inicial
    new_node->history.entry_count = 0;
    new_node->history.entries = (DAILY_HISTORY_ENTRY*)malloc(sizeof(DAILY_HISTORY_ENTRY) * 50);
    
    new_node->next = ht->table[index];
    ht->table[index] = new_node;
    
    return &new_node->history;
}

// Inicializa la estructura principal y las tablas hash
BIO_SIM_DATA* createBiosimData(int max_i, int max_t) {
    BIO_SIM_DATA *data = (BIO_SIM_DATA*)malloc(sizeof(BIO_SIM_DATA));
    if (!data) return NULL;

    // <--- CRÍTICO: Inicializar punteros a NULL para evitar Segmentation Faults en Algorithms.c --->
    data->eventQueue = NULL;
    data->activeInfectedIDs = NULL;
    data->infectedCount = 0;
    data->deathCount = 0;
    data->isolatedCount = 0;

    data->totalRisk = 0.0;

    data->activeStrainCount = 0;
    
    data->max_individuos = max_i;
    data->max_territorios = max_t;

    // Crear Tablas Hash
    data->cepas_hash_table = createStrainHashTable();
    data->persons_table = createPersonHashTable();
    data->regions_table = createRegionHashTable();

    // Inicializar la Tabla Hash de Historial
    data->history_table = createHistoryHashTable();

    // Verificación de memoria
    if (!data->cepas_hash_table || !data->persons_table || !data->regions_table) {
        if (data->cepas_hash_table) freeStrainInHash(data->cepas_hash_table);
        if (data->persons_table) freePersonInHash(data->persons_table);
        if (data->regions_table) freeRegionInHash(data->regions_table);
        free(data);
        return NULL;
    }

    return data;
}

// Liberar toda la memoria al cerrar
void free_biosim_data(BIO_SIM_DATA *data) {
    if (!data) return;
    if (data->cepas_hash_table) freeStrainInHash(data->cepas_hash_table);
    if (data->persons_table) freePersonInHash(data->persons_table);
    if (data->regions_table) freeRegionInHash(data->regions_table);
    
    // Liberar estructuras de simulación si se usaron
    // (Nota: freeMinHeap debe estar disponible si incluyes BinaryHeap.h o similar)
    // if (data->eventQueue) freeMinHeap(data->eventQueue); 
    //if (data->activeInfectedIDs) free(data->activeInfectedIDs);
    if (data->history_table) {
        for (int i = 0; i < PERSON_HASH_TABLE_SIZE; i++) {
            HISTORY_NODE *current = data->history_table->table[i];
            while (current != NULL) {
                HISTORY_NODE *temp = current;
                current = current->next;
                free(temp->history.entries); // Liberar el array dinámico de entradas
                free(temp);
            }
        }
        free(data->history_table);
    }
    
    //free(data);
}

// --------------------------------------------------------------------------------
//                          CARGA DE DATOS (DAO)
// --------------------------------------------------------------------------------

BIO_SIM_DATA* load_initial_data(const char *cepas_f, const char *terr_f, const char *ind_f, const char *cont_f, const char *reg_conn_f) {    const int DEFAULT_MAX_I = 10000;
    const int DEFAULT_MAX_T = 1000;
    BIO_SIM_DATA *data = createBiosimData(DEFAULT_MAX_I, DEFAULT_MAX_T);
    if (!data) return NULL;

    FILE *fp;

    // 1. CARGA DE CEPAS
    fp = fopen(cepas_f, "r");
    if (fp) {
        char line[256];
        printf(" / Loading strains from '%s'...\n", cepas_f);
        int cepa_count = 0;
        // Saltar header si existe (opcional, tu fgets lo lee como linea invalida si tiene texto)
        // fgets(line, sizeof(line), fp); 

        while (fgets(line, sizeof(line), fp)) {
            STRAIN s = {0};
            if (sscanf(line, "%d,%19[^,],%lf,%lf,%lf,%lf", &s.id, s.name, &s.beta, &s.caseFatalityRatio, &s.recovery, &s.mutationProb) >= 1) {
                insertStrainInHash(data->cepas_hash_table, &s);
                cepa_count++;
            }
        }
        fclose(fp);
        printf(" / Loaded %d strains.\n", cepa_count);
        data->activeStrainCount = cepa_count;
    } else {
        fprintf(stderr, "!!!Warning: Strains file '%s' not found. Continuing...\n", cepas_f);
    }

    // 2. CARGA DE REGIONES
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
                r.peopleIDs = NULL; // No usado en esta versión
                r.populationCount = 0;
                r.infectedCount = 0;
                insertRegionInHash(data->regions_table, &r);
                region_count++;
            }
        }
        fclose(fp);
        printf(" / Loaded %d regions.\n", region_count);
    } else {
        fprintf(stderr, "!!!Warning: Regions file '%s' not found. Continuing...\n", terr_f);
    }

    // 3. CARGA DE CONEXIONES ENTRE REGIONES
    if (reg_conn_f) {
        FILE *fr = fopen(reg_conn_f, "r");
        if (fr) {
            char line[256];
            printf(" / Loading region connections from '%s'...\n", reg_conn_f);
            while (fgets(line, sizeof(line), fr)) {
                int r1, r2;
                double dist;
                if (sscanf(line, "%d,%d,%lf", &r1, &r2, &dist) == 3) {
                    REGION *reg1 = searchRegionInHash(data->regions_table, r1);
                    REGION *reg2 = searchRegionInHash(data->regions_table, r2);
                    if (reg1 && reg2) {
                        addRegionConnection(reg1, r2, dist);
                        addRegionConnection(reg2, r1, dist); // Bidireccional
                    }
                }
            }
            fclose(fr);
        }
    }

    // 4. CARGA DE PERSONAS
    fp = fopen(ind_f, "r");
    if (fp) {
        char line[512];
        printf(" / Loading persons from '%s'...\n", ind_f);
        int person_count = 0;
        // fgets(line, sizeof(line), fp); // Header skip
        
        while (fgets(line, sizeof(line), fp)) {
            int id, regionID, status, actualStrainID, daysInfected;
            char name[128];
            double initialDegree, initialRisk;
            
            int scanned = sscanf(line, "%d,%127[^,],%d,%lf,%lf,%d,%d,%d", 
                &id, name, &regionID, &initialDegree, &initialRisk, &status, &actualStrainID, &daysInfected);
            
            if (scanned >= 3) {
                // createPerson inicializa el array estático de contactos a -1
                PERSON *p = createPerson(id, name, regionID, initialDegree, initialRisk, daysInfected);
                if(p) {
                    if (scanned < 6) status = HEALTH;
                    p->status = (HealthStatus)status;
                    p->actualStrainID = actualStrainID;
                    p->infectedBy = -1;
                    REGION *r = searchRegionInHash(data->regions_table, p->regionID);
                    r->populationCount++,
                    
                    insertPersonInHash(data->persons_table, p);
                    free(p); // Insert hace copia profunda, liberamos el temporal
                    person_count++;
                }
            }
        }
        fclose(fp);
        printf(" / Loaded %d persons.\n", person_count);
        data->max_individuos = person_count;
        data->max_individuos = person_count; // Actualizar límite real

        // --- AHORA SÍ: GENERAR EL ARREGLO LINEAL (Para el MergeSort) ---
        data->personArray = (PERSON**)malloc(sizeof(PERSON*) * person_count);
        
        // Llenar el arreglo recorriendo la tabla hash
        int idx = 0;
        for (int i = 0; i < PERSON_HASH_TABLE_SIZE; i++) {
            PERSON_NODE *node = data->persons_table->table[i];
            while (node != NULL) {
                if (idx < person_count) {
                    data->personArray[idx] = &node->data; // Guardamos el puntero a la persona real
                    idx++;
                }
                node = node->next;
            }
        }

        // Ordenar inmediatamente (Fase 1)
        printf(" / Sorting persons by Initial Risk (MergeSort)...\n");
        sortPersonArray(data); // Llamada a tu función corregida
    } else {
        fprintf(stderr, "!!!Warning: Persons file '%s' not found. Continuing...\n", ind_f);
    }

    // 5. CARGA DE CONTACTOS (GRAFO ESTÁTICO)
    if (cont_f) {
        FILE *fc = fopen(cont_f, "r");
        if (fc) {
            char line[256];
            printf(" / Loading contacts from '%s'...\n", cont_f);
            int contact_count = 0;
            
            // fgets(line, sizeof(line), fc); // Header skip

            while (fgets(line, sizeof(line), fc)) {
                int id1, id2;
                double weight = 0.5; // Valor por defecto (cercanía media)

                // Intentar leer: id1,id2,peso
                // Si el archivo solo tiene id1,id2, weight se queda en 0.5 o se hace random
                int matches = sscanf(line, "%d,%d,%lf", &id1, &id2, &weight);
                
                if (matches >= 2) { // Al menos leyó los IDs
                    if (matches == 2) {
                        // Si no viene peso en el TXT, generar aleatorio para realismo
                        // 10% muy cercanos (familia), 90% normales
                        weight = ((double)rand() / RAND_MAX); 
                    }
                    
                    PERSON *p1 = searchPersonInHash(data->persons_table, id1);
                    PERSON *p2 = searchPersonInHash(data->persons_table, id2);

                    if (p1 && p2) {
                        addContact(p1, id2, weight);
                        addContact(p2, id1, weight); // Grafo no dirigido
                        contact_count++;
                    }
                }
            }
            fclose(fc);
            printf(" / Loaded %d connections (edges).\n", contact_count);
        } else {
            fprintf(stderr, "!!!Warning: Contacts file '%s' not found.\n", cont_f);
        }
    }

    // --- RESUMEN FINAL DE CARGA ---
    printf("\n========== DATA LOAD SUMMARY ==========\n");
    printf("Strains loaded:     %d\n", data->cepas_hash_table->count);
    printf("Regions loaded:     %d\n", data->regions_table->count);
    printf("Persons loaded:     %d\n", data->persons_table->count);
    printf("======================================\n\n");

    /* Inicializar posiciones visuales (Counting Sort + Radial Layout) */
    initializePositions(data);

    return data;
}

// IMPLEMENTACIÓN DE CONSULTA O(1)
DAILY_HISTORY_ENTRY* get_history_by_id_and_day(BIO_SIM_DATA *data, int person_id, int day) {
    if (!data || !data->history_table || day <= 0) return NULL;

    // 1. O(1): Buscar el nodo de historial de la persona
    unsigned int index = hashFunction(person_id, PERSON_HASH_TABLE_SIZE); 
    HISTORY_NODE *current = data->history_table->table[index];

    while (current != NULL) {
        if (current->person_id == person_id) {
            PERSON_HISTORY *p_hist = &current->history;
            
            // 2. O(1): Acceder al array indexado por día (si la fecha es válida)
            // (La entrada del día 1 está en el índice 0; día N en índice N-1)
            int array_index = day - 1; 

            // Verificamos que el día solicitado ya se haya registrado
            if (array_index >= 0 && array_index < p_hist->entry_count) {
                // Verificamos que el día en la estructura coincida (seguridad extra)
                if (p_hist->entries[array_index].day == day) {
                    return &p_hist->entries[array_index];
                }
            }
            return NULL;
        }
        current = current->next;
    }
    return NULL;
}

// --------------------------------------------------------------------------------
//                          SISTEMA DE POSICIONES (VISUALIZACIÓN)
// --------------------------------------------------------------------------------

void initializePositions(BIO_SIM_DATA *data) {
    if (!data || !data->regions_table || !data->persons_table) return;

    REGION_HASH_TABLE *regions_table = data->regions_table;
    PERSON_HASH_TABLE *persons_table = data->persons_table;
    int num_regions = regions_table->count;
    if (num_regions == 0) return;

    printf("\n[Initializing] Region positions (Radial Layout)...\n");
    
    double angle_increment = 2.0 * 3.14159265359 / num_regions;
    double region_distance = 200.0;
    double region_radius = 30.0;
    int region_idx = 0;

    // Fase 1: Posicionar Regiones en círculo
    for (int i = 0; i < REGION_HASH_TABLE_SIZE; i++) {
        REGION_NODE *node = regions_table->table[i];
        while (node != NULL) {
            double angle = region_idx * angle_increment;
            node->data.drawConf.pos[0] = region_distance * cos(angle);
            node->data.drawConf.pos[1] = region_distance * sin(angle);
            node->data.drawConf.radio = region_radius;
            region_idx++;
            node = node->next;
        }
    }

    printf("[Initializing] Sorting persons (Counting Sort O(N))...\n");
    
    int total_persons = persons_table->count;
    if (total_persons == 0) return;

    // Fase 2: Counting Sort para agrupar personas por región
    
    // 2.1 Buscar región ID máxima para el tamaño del array de conteo
    int max_region_id = 0;
    for (int i = 0; i < PERSON_HASH_TABLE_SIZE; i++) {
        PERSON_NODE *tmp = persons_table->table[i];
        while (tmp) {
            if (tmp->data.regionID > max_region_id) max_region_id = tmp->data.regionID;
            tmp = tmp->next;
        }
    }

    // 2.2 Allocar memoria (Calloc inicializa en 0)
    PERSON *sorted_persons = (PERSON*)calloc(total_persons, sizeof(PERSON)); 
    int *count = (int*)calloc(max_region_id + 2, sizeof(int)); // Buffer de seguridad
    
    if (!sorted_persons || !count) {
        printf("Error: Memory allocation failed in initializePositions\n");
        if(sorted_persons) free(sorted_persons);
        if(count) free(count);
        return;
    }

    // 2.3 Contar frecuencias
    for (int i = 0; i < PERSON_HASH_TABLE_SIZE; i++) {
        PERSON_NODE *tmp = persons_table->table[i];
        while (tmp) {
            if (tmp->data.regionID >= 0 && tmp->data.regionID <= max_region_id) {
                count[tmp->data.regionID]++;
            }
            tmp = tmp->next;
        }
    }

    // 2.4 Calcular índices iniciales (Acumulado)
    int total = 0;
    for (int i = 0; i <= max_region_id; i++) {
        int temp = count[i];
        count[i] = total;
        total += temp;
    }

    // 2.5 Llenar array ordenado (Copia de estructuras)
    for (int i = 0; i < PERSON_HASH_TABLE_SIZE; i++) {
        PERSON_NODE *tmp = persons_table->table[i];
        while (tmp) {
            int rid = tmp->data.regionID;
            if (rid >= 0 && rid <= max_region_id) {
                int pos = count[rid]++;
                if (pos < total_persons) {
                    sorted_persons[pos] = tmp->data; 
                }
            }
            tmp = tmp->next;
        }
    }

    // Fase 3: Asignar posiciones aleatorias dentro de cada región
    printf("[Initializing] Assigning random positions within regions...\n");
    int current_region_id = -1;
    REGION *current_region = NULL;
    
    for (int i = 0; i < total_persons; i++) {
        PERSON *person = &sorted_persons[i];
        
        // Optimización: Solo buscar región si cambia (gracias al sort)
        if (person->regionID != current_region_id) {
            current_region_id = person->regionID;
            current_region = searchRegionInHash(regions_table, current_region_id);
        }

        // Posición aleatoria en coordenadas polares locales
        double r = ((double)rand() / RAND_MAX) * region_radius;
        double theta = ((double)rand() / RAND_MAX) * 2.0 * 3.14159265359;

        if (current_region) {
            person->drawConf.pos[0] = current_region->drawConf.pos[0] + r * cos(theta);
            person->drawConf.pos[1] = current_region->drawConf.pos[1] + r * sin(theta);
        }
    }

    // Fase 4: Actualizar la Hash Table original con las nuevas posiciones
    for (int i = 0; i < total_persons; i++) {
        PERSON *p_sort = &sorted_persons[i];
        PERSON *p_hash = searchPersonInHash(persons_table, p_sort->id);
        if (p_hash) {
            p_hash->drawConf = p_sort->drawConf;
        }
    }

    free(sorted_persons);
    free(count);
    printf("[Success] Positions initialized.\n");
}

// --------------------------------------------------------------------------------
//                          HELPERS Y WRAPPERS
// --------------------------------------------------------------------------------

STRAIN* get_cepa_by_id(BIO_SIM_DATA *data, int id) {
    if (!data) return NULL;
    return searchStrainInHash(data->cepas_hash_table, id);
}

PERSON* get_person_by_id(BIO_SIM_DATA *data, int id) {
    if (!data) return NULL;
    return searchPersonInHash(data->persons_table, id);
}

REGION* get_region_by_id(BIO_SIM_DATA *data, int id) {
    if (!data) return NULL;
    return searchRegionInHash(data->regions_table, id);
}

// Placeholder para historial
void save_contagion_history(BIO_SIM_DATA *data, int dia_simulacion) {
    if (!data || !data->history_table) return;

    // Iterar sobre personas para guardar estados relevantes (O(N) total)
    for (int i = 0; i < PERSON_HASH_TABLE_SIZE; i++) {
        PERSON_NODE *node = data->persons_table->table[i];
        while (node != NULL) {
            PERSON *p = &node->data;

            // Guardar solo si hay un estado relevante o es el día 1 (para trazabilidad)
            if (p->status != HEALTH || dia_simulacion == 1) { 
                
                PERSON_HISTORY *hist = get_or_create_history(data->history_table, p->id);
                
                // Redimensionar si es necesario (O(1) amortizado)
                if (hist->entry_count >= hist->capacity) {
                    hist->capacity *= 2;
                    hist->entries = (DAILY_HISTORY_ENTRY*)realloc(
                        hist->entries, 
                        hist->capacity * sizeof(DAILY_HISTORY_ENTRY)
                    );
                }

                // Guardar entrada en la siguiente posición
                DAILY_HISTORY_ENTRY *entry = &hist->entries[hist->entry_count];
                entry->day = dia_simulacion;
                entry->status = p->status;
                entry->strain_id = p->actualStrainID;
                hist->entry_count++;
            }
            node = node->next;
        }
    }
}