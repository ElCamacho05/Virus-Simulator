// Self Library
#include "Algorithms.h"

// General Libraries
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include "Clases/Person.h"
#include "Clases/Virus.h"
#include "../DAO_General.h"


// TYPES OF EVENTS (for patients and non-patients)
#define EVENT_RECOVERY 1
#define EVENT_DEATH 2

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

void add_to_active_infected(BIO_SIM_DATA *data, int person_id) {
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
            add_to_active_infected(data, p->id);

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
            
            remove_from_active_infected(data, p->id);
        }
    }

    // ============================================================
    // PARTE 2: PROPAGACIÓN DE CONTAGIOS (Grafo) - O(I * K)
    // ============================================================
    
    // Guardamos el conteo actual para NO iterar sobre los que se contagien HOY mismo
    // (simulamos que el contagio tarda al menos 1 día en ser infeccioso)
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

        // 2. Iterar sobre sus contactos directos (Lista de Adyacencia)
        ContactNode *contactoActual = spreader->contacts;
        
        while (contactoActual != NULL) {
            PERSON *target = contactoActual->contact; // La víctima potencial

            // Solo intentamos contagiar si la persona está SANA
            if (target->status == HEALTH) {
                
                // Cálculo de Probabilidad:
                // Factor Viral (Beta) * Factor de Contacto (si lo hubiera) * Aleatoriedad
                // Aquí usamos beta directo. Se puede multiplicar por contactoActual->interactionProb si lo usas.
                double probabilidadContagio = virus->beta;

                if (((double)rand() / RAND_MAX) < probabilidadContagio) {
                    // --- ¡NUEVO CONTAGIO CONFIRMADO! ---

                    // A. Actualizar Estado
                    target->status = INFECTED;
                    target->actualStrainID = virus->id;
                    target->daysInfected = 0;
                    target->infectedBy = spreader->id; // Guardamos quién lo contagió (Trazabilidad)

                    // B. Agregar a la lista de propagadores (para contagiar mañana)
                    add_to_active_infected(data, target->id);
                    
                    // printf("  [☣️ CONTAGIO] %s contagió a %s (Cepa: %s)\n", spreader->name, target->name, virus->name);

                    // C. Calcular su Destino (Recuperación o Muerte) y Agendar en el Heap
                    // Duración estimada basada en el virus (recovery rate * 100 es un ejemplo, adáptalo a tus datos)
                    int duracion = (int)(virus->recovery * 100); 
                    if (duracion < 2) duracion = 5; // Mínimo 5 días
                    
                    int dia_evento = dia_actual + duracion;
                    
                    // Determinar si morirá o se recuperará
                    int tipo_evento = EVENT_RECOVERY;
                    if (((double)rand() / RAND_MAX) < virus->caseFatalityRatio) {
                        tipo_evento = EVENT_DEATH;
                    }

                    // Insertar evento futuro en el Min-Heap (O(log N))
                    insertMinHeap(data->eventQueue, target->id, (double)dia_evento, tipo_evento);
                }
            }
            
            // Siguiente contacto
            contactoActual = contactoActual->next;
        }
        
        // Aumentar contador de días del infectado actual (opcional, ya que el Heap maneja el final)
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

void cluster_strains_by_name(BIO_SIM_DATA *data) {
    // Implementación de la construcción del Árbol Trie (O(N*L))
    (void)data;
    printf("[Tarea 7] Clustering inicializado. Construcción del Trie pendiente.\n");
}
