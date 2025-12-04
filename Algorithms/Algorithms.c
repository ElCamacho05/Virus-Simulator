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
#include "interface.h"

// TYPES OF EVENTS
#define EVENT_RECOVERY 1
#define EVENT_DEATH 2
#define EVENT_IMMUNITY_LOSS 3 // Nuevo evento para pérdida de inmunidad

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

/* ----------------------------------------------------------------------
   -----------------------     AUX ALGORITHMS     -----------------------
   ---------------------------------------------------------------------- */

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
    for (int i = 0; i < data->infectedCount; i++) {
        if (data->activeInfectedIDs[i] == person_id) {
            // Sobrescribir con el último y reducir tamaño
            data->activeInfectedIDs[i] = data->activeInfectedIDs[data->infectedCount - 1];
            data->infectedCount--;
            return;
        }
    }
}

/* ----------------------------------------------------------------------
   -----------------------    INITIAL OUTBREAK    -----------------------
   ---------------------------------------------------------------------- */

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

/* ----------------------------------------------------------------------
   -----------------------    DAILY SIMULATION    -----------------------
   ---------------------------------------------------------------------- */

void run_daily_simulation(BIO_SIM_DATA *data, int dia_actual) {
    
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
                // printf("  [NORMALIDAD] Persona %d vuelve a ser susceptible.\n", p->id);
            }
            continue;
        }

        // --- CASO: FIN DE INFECCIÓN ---
        if (p->status == INFECTED) {
            REGION *region = get_region_by_id(data, p->regionID);
            
            if (event.type == EVENT_DEATH) {
                p->status = DEATH;
                printf("  [DECESO] Persona %d (%s) ha fallecido.\n", p->id, p->name);
                
                // Actualizar contadores
                if (region) {
                    if (region->infectedCount > 0) region->infectedCount--;
                    region->deathCount++;
                }
                data->deathCount++;

            } else {
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
                    else factorRegional = 0.05; 
                }

                double prob = virus->beta * contactWeight * factorRegional;

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
                    drawInfectionLine(target, spreader);

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

    printf("=== FIN DIA %d | Infectados Activos: %d | Fallecidos: %d ===\n", 
           dia_actual, data->infectedCount, data->deathCount);
}

/* ----------------------------------------------------------------------
   -----------------------   HEAP IMPLEMENTATION  -----------------------
   ---------------------------------------------------------------------- */

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