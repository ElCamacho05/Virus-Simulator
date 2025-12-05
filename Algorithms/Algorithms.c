// Self Library
#include "Algorithms.h"

// General Libraries
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "Clases/Person.h"
#include "Clases/Virus.h"
#include "Clases/Regions.h"
#include "../DAO_General.h"
#include "dataRepository.h"
// #include "interface.h"

// TYPES OF EVENTS
#define EVENT_RECOVERY 1
#define EVENT_DEATH 2
#define EVENT_IMMUNITY_LOSS 3 // Nuevo evento para pérdida de inmunidad
#define EVENT_END_ISOLATION 4
#define EVENT_END_VACCINE_EFFECT 5

// Importamos mutate_strain de Virus.c
extern STRAIN* mutate_strain(STRAIN *parent, int new_id);

unsigned int hashFunction(int key, int size) {
    return (unsigned int)key % size;
}

/* ----------------------------------------------------------------------
   -----------------------  QUEUE IMPLEMENTATION  -----------------------
   (Necesaria para BFS)
   ---------------------------------------------------------------------- */

void enqueue(Queue*q, int id) {
    QueueNode *newNode =(QueueNode*)malloc(sizeof(QueueNode));
    if (!newNode) return;
    newNode->person_id= id;
    newNode->next = NULL;
    if(q->rear ==NULL){
        q->front = q->rear = newNode;
        return;
    }
    q->rear->next = newNode;
    q->rear = newNode;
}

int dequeue(Queue *q) {
    if (q->front == NULL) return -1;
    QueueNode *temp = q->front;
    int id = temp->person_id;
    q->front = q->front->next;
    if (q->front == NULL) q->rear = NULL;
    free(temp);
    return id;
}

/* 
----------------------------------------------------------------------
-----------------------     AUX ALGORITHMS     -----------------------
----------------------------------------------------------------------
*/

void analyze_connectivity_bfs(BIO_SIM_DATA *data, int start_person_id) {
    bool *visited = (bool*)calloc(data->max_individuos + 1, sizeof(bool));
    if (!visited) return;
    
    Queue q = {NULL, NULL};
    enqueue(&q, start_person_id);
    visited[start_person_id] = true;
    
    int total_affected_in_zone = 0;

    while (q.front != NULL) {
        int u_id = dequeue(&q);
        PERSON *u = get_person_by_id(data, u_id);
        if (u != NULL) {
            total_affected_in_zone++;
        }
    }
    while (q.front != NULL) dequeue(&q); 
    
    printf("  [BFS Analysis]: Zona de brote desde ID %d cubre %d individuos.\n", start_person_id, total_affected_in_zone);
    free(visited);
}

void add_to_active_infected(BIO_SIM_DATA *data, int person_id, char strainName[]) {
    printf("[INFECTED] - Person %d is now infected!!!\n", person_id);
    
    // 1. Agregar al Array de propagadores activos
    if (data->activeInfectedIDs && data->infectedCount < data->max_individuos) {
        data->activeInfectedIDs[data->infectedCount++] = person_id;
    }

    // 2. Actualizar estadística regional
    PERSON *person = get_person_by_id(data, person_id);
    if (person) {
        REGION *region = get_region_by_id(data, person->regionID);
        if (region) region->infectedCount++;
    }
}

void remove_from_active_infected(BIO_SIM_DATA *data, int person_id) {
    PERSON *person = get_person_by_id(data, person_id);
    if (!person) return;
    person->actualStrainID = -1;
    person->daysInfected = 0;
    person->infectedBy = -1;

    REGION *region = get_region_by_id(data, person->regionID);
    if (region) region->infectedCount--;

    for (int i = 0; i < data->infectedCount; i++) {
        if (data->activeInfectedIDs[i] == person_id) {
            // Sobrescribir con el último y reducir tamaño
            data->activeInfectedIDs[i] = data->activeInfectedIDs[data->infectedCount - 1];
            data->infectedCount--;
            return;
        }
    }
}

/* 
----------------------------------------------------------------------
-----------------------         SORTING        -----------------------
---------------------------------------------------------------------- 
*/

/*
------------
-- PERSON --
------------
*/
// merge sort (muchas personas)
void mergePerson(PERSON **arr, int l, int m, int r) {
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;

    // Crear arreglos temporales de PUNTEROS
    PERSON **L = (PERSON**)malloc(n1 * sizeof(PERSON*));
    PERSON **R = (PERSON**)malloc(n2 * sizeof(PERSON*));

    // Copiar datos a los temporales
    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    // Mezclar los temporales de vuelta en arr[l..r]
    i = 0; 
    j = 0; 
    k = l;
    while (i < n1 && j < n2) {
        // CRITERIO DE ORDENAMIENTO: Riesgo Descendente (> para mayor primero)
        if (L[i]->initialRisk >= R[j]->initialRisk) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    // Copiar los elementos restantes de L[], si hay
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    // Copiar los elementos restantes de R[], si hay
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }

    free(L);
    free(R);
}

void mergeSortPersonArrayRecursivo(PERSON **arr, int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;

        mergeSortPersonArrayRecursivo(arr, l, m);
        mergeSortPersonArrayRecursivo(arr, m + 1, r);

        mergePerson(arr, l, m, r);
    }
}

// public function
void sortPersonArray(BIO_SIM_DATA *data) {
    if (!data || !data->personArray || data->max_individuos <= 0) return;
    
    mergeSortPersonArrayRecursivo(data->personArray, 0, data->max_individuos - 1);
}

/*
------------
-- STRAIN --
------------
*/

// === UTILIDADES DE INTERCAMBIO ===
void swap_strain(STRAIN *a, STRAIN *b) {
    STRAIN temp = *a;
    *a = *b;
    *b = temp;
}

void swap_region(REGION *a, REGION *b) {
    REGION temp = *a;
    *a = *b;
    *b = temp;
}

// ======================================================================
// ===  HEAPSORT- Adaptado para crecimiento dinámico
// ======================================================================

// Estructura auxiliar para manejar el montículo
typedef struct {
    STRAIN *array;
    int n;
} StrainHeap;

void init_strain_heap(StrainHeap *h, int space) {
    // Alocación dinámica basada en el conteo ACTUAL
    h->array = (STRAIN *)malloc(sizeof(STRAIN) * space);
    h->n = 0;
}

// Push con "Flotar" (Max-Heap por ID)
void push_strain_heap(StrainHeap *h, STRAIN dato) {
    int i = h->n;
    h->array[i] = dato;
    h->n++;

    // Flotar
    for (; i > 0; i = (i - 1) / 2) {
        // Ordenamos por ID (puedes cambiar a 'beta' o 'letalidad' si prefieres)
        if (h->array[i].id > h->array[(i - 1) / 2].id) {
            swap_strain(&h->array[i], &h->array[(i - 1) / 2]);
        } else {
            break;
        }
    }
}

// Pop con "Hundir"
STRAIN pop_strain_heap(StrainHeap *h) {
    STRAIN temp = h->array[0];
    int i = 0;
    
    // Mover el último al principio
    h->array[0] = h->array[(h->n) - 1];
    h->n--;

    // Hundir
    int hMayor;
    while (2 * i + 1 < h->n) { 
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        
        if (right < h->n && h->array[right].id > h->array[left].id)
            hMayor = right;
        else
            hMayor = left;

        if (h->array[hMayor].id > h->array[i].id) {
            swap_strain(&h->array[i], &h->array[hMayor]);
            i = hMayor;
        } else {
            break;
        }
    }
    return temp;
}

// --- FUNCIÓN PRINCIPAL: HEAPSORT DINÁMICO ---
void sortStrainArray(BIO_SIM_DATA *data) {
    if (!data || !data->cepas_hash_table) return;

    // 1. OBTENER TAMAÑO REAL ACTUAL (Clave para evitar errores)
    int total_strains = data->cepas_hash_table->count;
    
    if (total_strains == 0) {
        printf("[Heapsort] No hay cepas registradas.\n");
        return;
    }

    printf("\n[Heapsort] Ordenando %d cepas detectadas (incluyendo mutaciones)...\n", total_strains);

    // 2. Inicializar Heap con el tamaño exacto necesario
    StrainHeap h;
    init_strain_heap(&h, total_strains);
    if (!h.array) return; // Error de memoria

    // 3. Extraer todas las cepas de la Hash Table al Heap
    for (int i = 0; i < VIRUS_HASH_TABLE_SIZE; i++) {
        STRAIN_NODE *current = data->cepas_hash_table->table[i];
        while (current != NULL) {
            push_strain_heap(&h, current->data);
            current = current->next;
        }
    }

    // 4. Extraer ordenado (Pop devuelve el mayor -> Orden Descendente)
    printf("--- Lista de Cepas Ordenada por ID (Desc) ---\n");
    for (int i = 0; i < total_strains; i++) {
        STRAIN s = pop_strain_heap(&h);
        printf("  ID: %d | Nombre: %s | Beta: %.2f | Mutación: %.3f\n", 
               s.id, s.name, s.beta, s.mutationProb);
    }
    printf("--------------------------------------------\n");

    // 5. Liberar memoria temporal
    free(h.array);
}

/*
------------
-- REGION --
------------
*/

// ======================================================================
// ===  QUICKSORT 
// ======================================================================

// Partición Lomuto adaptada a REGION
int partition_region(REGION *arr, int low, int high) {
    int pivot = arr[high].id; // Pivote: ID de la región
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) {
        // Orden Ascendente (ID menor primero)
        if (arr[j].id < pivot) {
            i++;
            swap_region(&arr[i], &arr[j]);
        }
    }
    swap_region(&arr[i + 1], &arr[high]);
    return (i + 1);
}

// Función recursiva
void quickSort_region_recursive(REGION *arr, int low, int high) {
    if (low < high) {
        int pi = partition_region(arr, low, high);
        quickSort_region_recursive(arr, low, pi - 1);
        quickSort_region_recursive(arr, pi + 1, high);
    }
}

// --- FUNCIÓN PRINCIPAL: QUICKSORT DINÁMICO ---
void sortRegionArray(BIO_SIM_DATA *data) {
    if (!data || !data->regions_table) return;

    // 1. Obtener conteo actual
    int total_regions = data->regions_table->count;
    if (total_regions == 0) return;

    // 2. Crear arreglo lineal temporal (Snapshot de los datos)
    REGION *region_array = (REGION *)malloc(sizeof(REGION) * total_regions);
    if (!region_array) return;

    // 3. Llenar arreglo desde la Hash Table
    int k = 0;
    for (int i = 0; i < REGION_HASH_TABLE_SIZE; i++) { 
        REGION_NODE *current = data->regions_table->table[i];
        while (current != NULL) {
            if (k < total_regions) {
                region_array[k++] = current->data;
            }
            current = current->next;
        }
    }

    printf("\n[Quicksort] Ordenando %d regiones por ID...\n", total_regions);

    // 4. Ejecutar Quicksort
    quickSort_region_recursive(region_array, 0, total_regions - 1);

    // 5. Mostrar Resultado
    for (int i = 0; i < total_regions; i++) {
        printf("  Region ID: %d | %s | Población: %d | Infectados: %d\n", 
               region_array[i].id, region_array[i].name, 
               region_array[i].populationCount, region_array[i].infectedCount);
    }

    // 6. Liberar memoria temporal
    free(region_array);
}

/* 
----------------------------------------------------------------------
-----------------------    INITIAL OUTBREAK    -----------------------
----------------------------------------------------------------------
*/

void establish_initial_outbreak(BIO_SIM_DATA *data, int num_brotes, int cepa_id) {
    // Inicialización de estructuras si no existen
    if (!data->eventQueue) {
        data->eventQueue = createMinHeap(data->max_individuos * 4); // Capacidad amplia para eventos futuros
        data->activeInfectedIDs = (int*)malloc(data->max_individuos * sizeof(int));
        data->infectedCount = 0;
        data->deathCount = 0;
    }

    STRAIN *virus = get_cepa_by_id(data, cepa_id);
    int count = 0;

    printf("[Brotes] Infectando %d pacientes cero...\n", num_brotes);

    while (count < num_brotes) {
        int random_id = (rand() % data->max_individuos) + 1;
        PERSON *p = get_person_by_id(data, random_id);

        if (p && p->status == HEALTH) {
            p->status = INFECTED;
            p->actualStrainID = cepa_id;
            p->daysInfected = 0;

            add_to_active_infected(data, p->id, virus->name);

            int duracion = (int)(virus->recovery * 100);
            if (duracion <= 0) duracion = 7;

            int type = EVENT_RECOVERY;
            if (((double)rand() / RAND_MAX) < virus->caseFatalityRatio) {
                type = EVENT_DEATH;
            }

            insertMinHeap(data->eventQueue, p->id, (double)duracion, type);
            count++;
        }
    }
}

/*
----------------------------------------------------------------------
-----------------------    DAILY SIMULATION    -----------------------
----------------------------------------------------------------------
*/

void run_daily_simulation(BIO_SIM_DATA *data, int dia_actual) {
    
    // always generates a new random seed
    if (data) {
        // Actualizar estado del nuevo infectado
        int randomPersonId = (rand() % data->max_individuos);
        int randomStrainId = (rand() % data->activeStrainCount);

        PERSON *target = get_person_by_id(data, randomPersonId);
        STRAIN *virus = get_cepa_by_id(data, randomStrainId);

        if (!target || !virus) goto a;
        target->status = INFECTED;
        target->actualStrainID = randomStrainId;
        target->daysInfected = 0;
        target->infectedBy = -1; 

        add_to_active_infected(data, target->id, virus->name);
        // drawInfectionLine(target, spreader);

        // Agendar evento de terminación (Recovery/Death)
        int dur = (int)(virus->recovery * 100); 
        if (dur < 2) dur = 5; 
        int type = (((double)rand() / RAND_MAX) < virus->caseFatalityRatio) ? EVENT_DEATH : EVENT_RECOVERY;

        insertMinHeap(data->eventQueue, target->id, (double)(dia_actual + dur), type);
    }

    a: {}

    // === PARTE 1: PROCESAR EVENTOS (Recuperaciones, Muertes y Pérdida de Inmunidad) ===
    while (!isHeapEmpty(data->eventQueue)) {
        // Ver si el evento más próximo ya ocurrió
        if (data->eventQueue->array[0].value > dia_actual) break;

        HeapNode event = extractMinHeap(data->eventQueue);
        PERSON *p = get_person_by_id(data, event.id);
        
        if (!p) continue;

        // --- CASO: PÉRDIDA DE INMUNIDAD (Vuelta a la normalidad) ---
        if (event.type == EVENT_IMMUNITY_LOSS) {
            if (p->status == IMMUNE) {
                p->status = HEALTH;
                // Reset total para que pueda volver a enfermarse como nuevo
                p->actualStrainID = -1;
                p->infectedBy = -1;
                p->daysInfected = 0;
                printf("  [NORMALIDAD] Persona %d vuelve a ser susceptible.\n", p->id);
            }
            continue;
        }
        
        if (event.type == EVENT_END_ISOLATION) {
            if (p->status == ISOLATED) {
                p->status = HEALTH; // Vuelve a estar sano
                data->isolatedCount--; // Decremento global
                printf("  [CUARENTENA FINALIZADA] Persona %d es liberada y esta sana.\n", p->id);
            }
            continue;
        }

        // --- CASO: FIN DE EFECTO DE VACUNA (NUEVO) ---
        if (event.type == EVENT_END_VACCINE_EFFECT) {
            if (p->status == VACCINATED) {
                p->status = HEALTH; // Vuelve a estar sano
                p->initialRisk *= 2.0; // El riesgo vuelve a la normalidad (0.5 * 2 = 1.0)
                // printf("  [EFECTO VACUNA PERDIDO] Persona %d es liberada y pierde proteccion.\n", p->id);
            }
            continue;
        }


        // --- CASO: FIN DE INFECCIÓN ---
        

        if (p->status == INFECTED) {
            REGION *region = get_region_by_id(data, p->regionID);
            
            if (event.type == EVENT_DEATH) {
                p->status = DEATH;
                printf("!!![DECESO] Persona %d (%s) ha fallecido.\n", p->id, p->name);
                
                // Actualizar contadores
                if (region) {
                    if (region->infectedCount > 0) region->infectedCount--;
                    region->deathCount++;
                }
                data->deathCount++;

            } else { // EVENT_RECOTERY
                // RECUPERACIÓN
                p->status = IMMUNE;
                printf("  [RECUPERADO] Persona %d (%s) es ahora inmune.\n", p->id, p->name);
                
                if (region) {
                    if (region->infectedCount > 0) region->infectedCount--;
                }

                // --- RESETEO DE VALORES DEL VIRUS ---
                p->actualStrainID = -1;
                p->daysInfected = 0;
                p->infectedBy = -1;

                // lower risk by 5%
                p->initialRisk = p->initialRisk*0.95;

                // --- PROGRAMAR PÉRDIDA DE INMUNIDAD ---
                // Duración aleatoria de inmunidad entre 20 y 50 días
                int dias_inmune = 20 + (rand() % 31);
                insertMinHeap(data->eventQueue, p->id, (double)(dia_actual + dias_inmune), EVENT_IMMUNITY_LOSS);
            }
            
            // Dejar de procesarlo como propagador activo
            remove_from_active_infected(data, p->id);
        }
    }

    // === PARTE 2: PROPAGACIÓN (Grafo Estático) ===
    int current_infected_count = data->infectedCount; 
    
    for (int i = 0; i < current_infected_count; i++) {
        int spreader_id = data->activeInfectedIDs[i];
        PERSON *spreader = get_person_by_id(data, spreader_id);
        
        if (!spreader || spreader->status != INFECTED) continue;

        STRAIN *virus = get_cepa_by_id(data, spreader->actualStrainID);
        if (!virus) continue;

        // Recorrer contactos (usando ARRAY ESTÁTICO)
        for (int j = 0; j < spreader->numContacts; j++) {
            if (spreader->contacts[j].contactID <= 0) continue;

            int targetID = spreader->contacts[j].contactID;
            double contactWeight = spreader->contacts[j].weight;
            PERSON *target = get_person_by_id(data, targetID);

            if (target && target->status == HEALTH) {
                
                // Cálculo de Factor Regional (Distancia)
                double factorRegional = 1.0;
                if (spreader->regionID != target->regionID) {
                    REGION *rOrigin = get_region_by_id(data, spreader->regionID);
                    double distancia = -1.0;
                    for(int k=0; k < rOrigin->numConnections; k++) {
                        if(rOrigin->connections[k].targetRegionId == target->regionID) {
                            distancia = rOrigin->connections[k].distanceKM;
                            break;
                        }
                    }
                    if (distancia > 0) factorRegional = 1.0 / (1.0 + (distancia / 500.0));
                    else factorRegional = 0.05; // las personas pueden volar, uno nunca sabe
                }
                double personRiskFactor = (1+target->initialRisk)/2.0;
                double prob = virus->beta * contactWeight * factorRegional * personRiskFactor;

                // Intento de contagio
                if (((double)rand() / RAND_MAX) < prob) {
                    
                    int finalStrainID = virus->id;
                    
                    // Mutación
                    if (((double)rand() / RAND_MAX) < virus->mutationProb) {
                        static int mutation_id_counter = 100; 
                        mutation_id_counter++;
                        STRAIN *mutant = mutate_strain(virus, mutation_id_counter);
                        
                        if (mutant) {
                            printf("  [MUTACION] Cepa %s mutó a -> %s (ID: %d)\n", virus->name, mutant->name, mutant->id);
                            
                            data->activeStrainCount++;
                            insertStrainInHash(data->cepas_hash_table, mutant);
                            
                            // Actualizar TRIE si existe (usamos la variable global definida al final)
                            extern struct TrieNode *strain_trie_root;
                            if (strain_trie_root) {
                                char cleanName[50];
                                int k = 0;
                                for (int idx = 0; mutant->name[idx] != '\0' && k < 49; idx++) {
                                    char c = mutant->name[idx];
                                    if (isalpha(c)) cleanName[k++] = tolower(c);
                                }
                                cleanName[k] = '\0';
                                insert(strain_trie_root, cleanName); 
                            }
                            finalStrainID = mutant->id; 
                            free(mutant); 
                        }
                    }

                    // Actualizar estado del nuevo infectado
                    target->status = INFECTED;
                    target->actualStrainID = finalStrainID;
                    target->daysInfected = 0;
                    target->infectedBy = spreader->id; 

                    add_to_active_infected(data, target->id, virus->name);
                    // drawInfectionLine(target, spreader);

                    // Agendar evento de terminación (Recovery/Death)
                    int dur = (int)(virus->recovery * 100); 
                    if (dur < 2) dur = 5; 
                    int type = (((double)rand() / RAND_MAX) < virus->caseFatalityRatio) ? EVENT_DEATH : EVENT_RECOVERY;

                    insertMinHeap(data->eventQueue, target->id, (double)(dia_actual + dur), type);
                }
            }
        }
        spreader->daysInfected++;
    }

    printf("\n\n=== FIN DIA %d | Infectados Activos: %d | Fallecidos: %d ===\n\n\n", 
           dia_actual, data->infectedCount, data->deathCount);
}

/*
----------------------------------------------------------------------
-----------------------    RISK MINIMIZING     -----------------------
----------------------------------------------------------------------
*/

// --- TAREA 4: MINIMIZACIÓN DE RIESGO (GREEDY) ---
// Complejidad: O(N) de ejecución (después del O(N log N) de pre-proceso)
void minimize_total_risk(BIO_SIM_DATA *data, double reduction_target_percentage, int mode) {
    if (!data || !data->personArray || data->max_individuos == 0) return;

    printf("\n[Tarea 4] Ejecutando algoritmo Greedy de aislamiento...\n");

    // 1. Calcular Límite Máximo de Personas a Aislar
    int maxIsolatedLimit = (int)(data->max_individuos * reduction_target_percentage);
    
    // 2. Variables de control
    double currentEliminatedRisk = 0.0;
    int newlyIsolatedCount = 0;

    int isoCount = 0;

    // 3. SELECCIÓN VORAZ (Greedy Selection)
    // Recorremos el arreglo ya ordenado de MAYOR RIESGO a MENOR.
    for (int i = 0; i < data->max_individuos; i++) {
        PERSON *p = data->personArray[i];

        // --- CONDICIÓN DE TOPE DE CAPACIDAD (NUEVO) ---
        // Si ya tenemos el número máximo de personas en aislamiento, NO aislamos más.
        if (isoCount >= maxIsolatedLimit) break;

        if (p->status == DEATH || p->status == ISOLATED || p->status == VACCINATED) {
            continue; 
        }

        // Solo aislamos si la persona está Activa (Sana, Infectada o Inmune) y NO ha sido aislada.
        // Si estaba infectado, sacarlo de la lista de propagación activa
        if (p->status == INFECTED) {
            remove_from_active_infected(data, p->id);
        }

        if (mode == ISOLATED) { 
            // 3.1a Aplicar aislamiento
            p->status = ISOLATED;
            insertMinHeap(data->eventQueue, p->id, (double)(simulation_day + 20), EVENT_END_ISOLATION);
        }
            
        if (mode == VACCINATED) { 
            // 3.1b aplicar vacuna
            p->status = VACCINATED;
            insertMinHeap(data->eventQueue, p->id, (double)(simulation_day + 50), EVENT_END_VACCINE_EFFECT);
            p->initialRisk *= 0.5; // drastically reduced risk
        }
        

        // 3.2 Programar fin de la cuarentena (10 días)
        // Se asume que esta función se llama al inicio del día (día actual)
        
        // 3.3 Actualizar Contadores Globales
        currentEliminatedRisk += p->initialRisk;
        data->isolatedCount++; // Incremento global
        isoCount++;
        newlyIsolatedCount++;

    }
    printf("%d\n", data->max_individuos);
    printf("%d\n", maxIsolatedLimit);

    printf("  - Límite de Aislamiento: %d personas (%.2f%%)\n", maxIsolatedLimit, reduction_target_percentage * 100);
    if (mode == ISOLATED)
        printf("  - Se aislaron %d nuevas personas de alto riesgo.\n", newlyIsolatedCount);
    else if (mode == VACCINATED) 
        printf("  - Se vacunaron %d nuevas personas de alto riesgo.\n", newlyIsolatedCount);

        // Nota: El riesgo total (totalRisk) se calcula en otra fase, aquí solo reportamos el riesgo de los que aislamos hoy.
    printf("  - Riesgo acumulado eliminado hoy: %.2f\n", currentEliminatedRisk);
}

/*
----------------------------------------------------------------------
-----------------------   HEAP IMPLEMENTATION  -----------------------
----------------------------------------------------------------------
*/

MinHeap* createMinHeap(int capacity) {
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
    if (!heap) return NULL;
    heap->size = 0;
    heap->capacity = capacity;
    heap->array = (HeapNode*)malloc(capacity * sizeof(HeapNode));
    return heap;
}

void swap(HeapNode *a, HeapNode *b) {
    HeapNode temp = *a;
    *a = *b;
    *b = temp;
}

void minHeapify(MinHeap* heap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < heap->size && heap->array[left].value < heap->array[smallest].value)
        smallest = left;

    if (right < heap->size && heap->array[right].value < heap->array[smallest].value)
        smallest = right;

    if (smallest != idx) {
        swap(&heap->array[idx], &heap->array[smallest]);
        minHeapify(heap, smallest);
    }
}

int isHeapEmpty(MinHeap* heap) {
    return !heap || heap->size == 0;
}

void insertMinHeap(MinHeap* heap, int id, double value, int type) {
    if (!heap) return;
    if (heap->size == heap->capacity) {
        heap->capacity *= 2;
        heap->array = realloc(heap->array, heap->capacity * sizeof(HeapNode));
        if (!heap->array) return; 
    }

    int i = heap->size++;
    heap->array[i].id = id;
    heap->array[i].value = value;
    heap->array[i].type = type;

    while (i && heap->array[(i - 1) / 2].value > heap->array[i].value) {
        swap(&heap->array[i], &heap->array[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

HeapNode extractMinHeap(MinHeap* heap) {
    HeapNode errorNode = {-1, -1.0, -1};
    if (isHeapEmpty(heap)) return errorNode;

    if (heap->size == 1) {
        heap->size--;
        return heap->array[0];
    }

    HeapNode root = heap->array[0];
    heap->array[0] = heap->array[heap->size - 1];
    heap->size--;
    minHeapify(heap, 0);

    return root;
}

void freeMinHeap(MinHeap* heap) {
    if(heap) {
        if(heap->array) free(heap->array);
        free(heap);
    }
}

// -------------------------------------------------------------
// --- (Punto 7): CLUSTERING DE CEPAS (Placeholder) ---
// -------------------------------------------------------------
/*
--------------------------------
            TRIE
--------------------------------
*/
//Declaracion global para la raiz del trie

struct TrieNode *strain_trie_root = NULL;

void cluster_strains(BIO_SIM_DATA *data) {
    if (!data || !data->cepas_hash_table) return;

    if(strain_trie_root == NULL){
        // createNode está definida en Virus.c, pero si no se encuentra el símbolo,
        // esta llamada asume que está disponible en el linkeo.
        // Para seguridad, nos basamos en que Virus.h expone las funciones del Trie.
        strain_trie_root = createNode(); 
    }
    printf("\n[Tarea 7] Construyendo el arbol TRIE para su clasificacion de cepas....\n");

    for(int i=0; i<VIRUS_HASH_TABLE_SIZE; i++){
        STRAIN_NODE *strain_node = data->cepas_hash_table->table[i];
        while(strain_node != NULL){
            STRAIN *s = &strain_node->data;
            // Inserción en el TRIE (la función insert está en Virus.c)
            insert(strain_trie_root, s->name);
            strain_node = strain_node->next;
        }
    }

    printf("[Tarea 7] Construccion del TRIE completada.\n");
}