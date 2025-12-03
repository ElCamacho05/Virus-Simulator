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
#include "../DAO_General.h"
#include "dataRepository.h"


// TYPES OF EVENTS (for patients and non-patients)
#define EVENT_RECOVERY 1
#define EVENT_DEATH 2

extern STRAIN* mutate_strain(STRAIN *parent, int new_id);

unsigned int hashFunction(int key, int size) {
    return (unsigned int)key % size;
}

//PUNTO 2

/*
--------------------------------
            QUEUE
--------------------------------
*/

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

// ----------------------------------------------------------------------
// ---------                AUX ALGORITHMS                      ---------
// ----------------------------------------------------------------------
void analyze_connectivity_bfs(BIO_SIM_DATA *data, int start_person_id) {
    // para inicializar el array de visitados, o usar un Hash Set para visitados.
    // Usamos el max_individuos definido en el DAO como proxy.
    bool *visited = (bool*)calloc(data->max_individuos + 1, sizeof(bool));
    if (!visited) return;
    
    Queue q = {NULL, NULL};
    enqueue(&q, start_person_id);
    visited[start_person_id] = true;
    
    int total_affected_in_zone = 0;

    while (q.front != NULL) {
        int u_id = dequeue(&q);
        PERSON *u = get_person_by_id(data, u_id); // Acceso O(1)

        if (u != NULL) {
            total_affected_in_zone++;
            
            // TODO: Implementar la iteración sobre la lista de contactos de 'u'
            // (ContList *directory) para ejecutar el BFS de la Tarea 2.
            
            /*
            for (each contact v of u) {
                if (!visited[v_id]) {
                    visited[v_id] = true;
                    enqueue(&q, v_id);
                }
            }
            */
        }
    }
    // Liberación de memoria de la cola (por si quedaron elementos)
    while (q.front != NULL) dequeue(&q); 
    
    printf("  [BFS Analysis]: Zona de brote conectada desde ID %d cubre %d individuos.\n", start_person_id, total_affected_in_zone);
    free(visited);
}

void add_to_active_infected(BIO_SIM_DATA *data, int person_id, char strainName[]) {
    printf("[INFECTED] - Person %d is now infected!!!\n", person_id);
    if (data->activeInfectedIDs && data->infectedCount < data->max_individuos) {
        data->activeInfectedIDs[data->infectedCount++] = person_id;
    }
}

void remove_from_active_infected(BIO_SIM_DATA *data, int person_id) {
    // Sequential search
    for (int i = 0; i < data->infectedCount; i++) {
        if (data->activeInfectedIDs[i] == person_id) {
            // Reemplazamos el eliminado por el último de la lista y reducimos el tamaño
            data->activeInfectedIDs[i] = data->activeInfectedIDs[data->infectedCount - 1];
            data->infectedCount--;
            return;
        }
    }
}

//PUNTO 2 (brotes aleatorios)
/*
--------------------------------
        INITIAL OUTBREAK
--------------------------------
*/

void establish_initial_outbreak(BIO_SIM_DATA *data, int num_brotes, int cepa_id) {
    if (!data->eventQueue) {
        // Inicializamos el heap si no existe (capacidad = max personas)
        data->eventQueue = createMinHeap(data->max_individuos);
        // Inicializamos la lista de infectados
        data->activeInfectedIDs = (int*)malloc(data->max_individuos * sizeof(int));
        data->infectedCount = 0;
    }

    STRAIN *virus = get_cepa_by_id(data, cepa_id);
    int infected_count = 0;

    printf("[Brotes] Infectando %d pacientes cero...\n", num_brotes);

    while (infected_count < num_brotes) {
        int random_id = (rand() % data->max_individuos) + 1;
        PERSON *p = get_person_by_id(data, random_id);

        if (p && p->status == HEALTH) {
            p->status = INFECTED;
            p->actualStrainID = cepa_id;
            p->daysInfected = 0;

            // 1. Agregar a lista de propagadores activos
            add_to_active_infected(data, p->id, virus->name);

            // 2. Calcular cuándo se recupera o muere (Lógica de Eventos)
            // Usamos la tasa de recuperación del virus para predecir el día
            int days_to_outcome = (int)(virus->recovery * 100); // Ejemplo simple
            if (days_to_outcome <= 0) days_to_outcome = 7;      // Default 7 días

            // Decidir destino fatal o recuperación ahora para agendar el evento
            int type = EVENT_RECOVERY;
            if (((double)rand() / RAND_MAX) < virus->caseFatalityRatio) {
                type = EVENT_DEATH;
            }

            // INSERTAR EN HEAP (O(log N))
            // El evento ocurrirá en el día actual + days_to_outcome
            insertMinHeap(data->eventQueue, p->id, (double)days_to_outcome, type);

            infected_count++;
        }
    }
}


//PUNTO 3(Simulacion de los contagios diarios)
/*
--------------------------------
        DAILY SIMULATION
--------------------------------
*/
// --- TAREA 3: SIMULACIÓN DIARIA ---
// Complejidad: O(I * K) para contagios + O(E log N) para eventos
// I = Infectados, K = Contactos promedio, E = Eventos de hoy
void run_daily_simulation(BIO_SIM_DATA *data, int dia_actual) {
    
    // ============================================================
    // PARTE 1: PROCESAR EVENTOS DEL DÍA (Heap) - O(E log N)
    // ============================================================
    // Sacamos del heap a todos los que les "toca" recuperarse o morir hoy
    
    while (!isHeapEmpty(data->eventQueue)) {
        // Miramos el tope sin sacar para ver si la fecha ya llegó
        if (data->eventQueue->array[0].value > dia_actual) {
            break; // El evento más próximo es en el futuro, terminamos por hoy.
        }

        // Sacamos el evento de hoy
        HeapNode event = extractMinHeap(data->eventQueue);
        PERSON *p = get_person_by_id(data, event.id);

        // Solo procesamos si la persona sigue infectada (por si hubo lógica extraña)
        if (p && p->status == INFECTED) {
            if (event.type == EVENT_DEATH) {
                p->status = DEATH;
                printf("  [DECESO] Persona %d (%s) ha fallecido.\n", p->id, p->name);
            } else {
                p->status = IMMUNE;
                printf("  [RECUPERADO] Persona %d (%s) es ahora inmune.\n", p->id, p->name);
            }
            
            // IMPORTANTE: Ya no contagia, lo sacamos de la lista de infectados activos
            remove_from_active_infected(data, p->id);
        }
    }

    // ============================================================
    // PARTE 2: PROPAGACIÓN DE CONTAGIOS (Grafo Estático) - O(I * K)
    // ============================================================
    
    // Guardamos el conteo actual para NO iterar sobre los que se contagien HOY mismo
    int current_infected_count = data->infectedCount; 
    
    for (int i = 0; i < current_infected_count; i++) {
        // 1. Obtener al propagador (Spreader)
        int spreader_id = data->activeInfectedIDs[i];
        PERSON *spreader = get_person_by_id(data, spreader_id);
        
        // Seguridad: si se curó justo hoy o no existe, saltar
        if (!spreader || spreader->status != INFECTED) continue;

        // Obtener datos del virus que porta
        STRAIN *virus = get_cepa_by_id(data, spreader->actualStrainID);
        if (!virus) continue;

        // 2. Iterar sobre sus contactos (ARRAY ESTÁTICO)
        for (int j = 0; j < spreader->numContacts; j++) {
            int targetID = spreader->contacts[j].contactID; // <--- CAMBIO
            double contactWeight = spreader->contacts[j].weight; // <--- NUEVO: Cercanía

            PERSON *target = get_person_by_id(data, targetID);

            // Solo intentamos contagiar si la persona existe y está SANA
            if (target && target->status == HEALTH) {
                // FORMULA MEJORADA DE CONTAGIO:
                // Probabilidad Base (Virus) * Factor de Cercanía (Relación)
                // Ejemplo: Virus 70% * Amigo Lejano (0.1) = 7% probabilidad real
                //          Virus 70% * Esposa (0.9) = 63% probabilidad real
                
                double probabilidadReal = virus->beta * contactWeight;

                // Opcional: Factor de ajuste si queda muy bajo
                // probabilidadReal = virus->beta * (0.3 + 0.7 * contactWeight);

                if (((double)rand() / RAND_MAX) < probabilidadReal) {   
                    // --- ¡NUEVO CONTAGIO CONFIRMADO! ---
                    
                    int finalStrainID = virus->id; 

                    // === LÓGICA DE MUTACIÓN ===
                    if (((double)rand() / RAND_MAX) < virus->mutationProb) {
                        static int mutation_id_counter = 100; 
                        mutation_id_counter++;

                        STRAIN *mutant = mutate_strain(virus, mutation_id_counter);
                        
                        if (mutant) {
                            printf("  [🧬 MUTACIÓN] Cepa %s mutó a -> %s (ID: %d)\n", virus->name, mutant->name, mutant->id);
                            
                            // Guardar en DAO
                            insertStrainInHash(data->cepas_hash_table, mutant);
                            
                            // Guardar en TRIE
                            extern struct TrieNode *strain_trie_root;
                            if (strain_trie_root) {
                                char cleanName[50];
                                int k = 0;
                                for (int idx = 0; mutant->name[idx] != '\0' && k < 49; idx++) {
                                    char c = mutant->name[idx];
                                    if (isalpha(c)) {
                                        cleanName[k++] = tolower(c);
                                    }
                                }
                                cleanName[k] = '\0';
                                insert(strain_trie_root, cleanName); 
                            }

                            finalStrainID = mutant->id; 
                            free(mutant); 
                        }
                    }

                    // A. Actualizar Estado
                    target->status = INFECTED;
                    target->actualStrainID = finalStrainID;
                    target->daysInfected = 0;
                    target->infectedBy = spreader->id; 

                    // B. Agregar a la lista de propagadores (contagiará mañana)
                    STRAIN *strain = get_cepa_by_id(GlobalData, spreader->actualStrainID);
                    add_to_active_infected(data, target->id, strain->name);
                    
                    // C. Calcular su Destino (Recuperación o Muerte)
                    int duracion = (int)(virus->recovery * 100); 
                    if (duracion < 2) duracion = 5; 
                    
                    int dia_evento = dia_actual + duracion;
                    
                    int tipo_evento = EVENT_RECOVERY;
                    if (((double)rand() / RAND_MAX) < virus->caseFatalityRatio) {
                        tipo_evento = EVENT_DEATH;
                    }

                    insertMinHeap(data->eventQueue, target->id, (double)dia_evento, tipo_evento);
                }
            }
        }
        
        // Aumentar contador de días del infectado actual
        spreader->daysInfected++;
    }
}

MinHeap* createMinHeap(int capacity) {
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
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

/*
--------------------------------
             HEAP
--------------------------------
*/

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
    return heap->size == 0;
}

void insertMinHeap(MinHeap* heap, int id, double value, int type) {
    if (heap->size == heap->capacity) {
        printf("Heap overflow\n");
        return;
    }

    int i = heap->size++;
    heap->array[i].id = id;
    heap->array[i].value = value;
    heap->array[i].type = type;

    // Flotar hacia arriba (Bubble up)
    while (i && heap->array[(i - 1) / 2].value > heap->array[i].value) {
        swap(&heap->array[i], &heap->array[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

HeapNode extractMinHeap(MinHeap* heap) {
    if (isHeapEmpty(heap)) {
        HeapNode errorNode = {-1, -1.0, -1};
        return errorNode;
    }

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
    // Implementación de la construcción del Árbol Trie (O(N*L))
    if (!data || !data->cepas_hash_table) return;

    // Se inicializa la raiz del TRIE (en caso de que no exista)
    if(strain_trie_root == NULL){
        strain_trie_root = createNode();
    }
    printf("\n[Tarea 7] Construyendo el arbol TRIE para su clasificacion de cepas....\n");

    // Interacion sobre la tabla hash de las cepas
    for(int i=0; i<VIRUS_HASH_TABLE_SIZE; i++){
        STRAIN_NODE *strain_node = data->cepas_hash_table->table[i];

        while(strain_node != NULL){
            STRAIN *s =&strain_node->data;

            insert(strain_trie_root, s->name);

            strain_node = strain_node->next;
        }
    }

    printf("\n[Tarea 7] Construyendo del TRIE completada. %d cepas clasificadas\n", data->cepas_hash_table->count);
}