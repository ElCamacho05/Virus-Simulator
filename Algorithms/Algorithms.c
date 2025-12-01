// General Libraries

// Self Library
#include "Algorithms.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

unsigned int hashFunction(int key, int size) {
    return (unsigned int)key % size;
}

//PUNTO 2

//Estructura de cola para BFS
typedef struct QueueNode{
    int person_id;
    struct QueueNode *next;
}QueueNode;

typedef struct{
    QueueNode *front;
    QueueNode *rear;
}Queue;

void enqueue(Queue*q, int id){
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
// --- ALGORITMO AUXILIAR: BFS PARA DETECCIÓN DE ZONAS (O(V+E)) ---
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


// ------------------------------------------------------------------------
// --- (Punto 2): ESTABLECER BROTES ALEATORIOS INICIALES ---
// ------------------------------------------------------------------------

void establish_initial_outbreak(BIO_SIM_DATA *data, int num_brotes, int cepa_id) {
    srand(time(NULL));
    int brotes_establecidos = 0;
    
    printf("\n[Tarea 2] Estableciendo %d brotes iniciales (Cepa ID: %d)...\n", num_brotes, cepa_id);
    
    int *infected_seeds = (int*)malloc(num_brotes * sizeof(int));
    if (!infected_seeds) return;

    // 1. SELECCIÓN ALEATORIA DE 10 INDIVIDUOS (O(1) promedio de búsqueda)
    while (brotes_establecidos < num_brotes && data->persons_table->count > 0) {
        int random_id = (rand() % data->max_individuos) + 1; 
        PERSON *p = get_person_by_id(data, random_id); 
        
        if (p != NULL && p->status == HEALTH) {
            p->status = INFECTED;
            p->actualStrainID = cepa_id;
            p->daysInfected = 1;
            infected_seeds[brotes_establecidos] = p->id;
            brotes_establecidos++;
        }
    }

    // 2. DETECCIÓN DE ZONAS CONECTADAS (Análisis de BFS)
    printf("Realizando análisis inicial de zonas conectadas (BFS). \n");
    for (int i = 0; i < brotes_establecidos; i++) {
        // Ejecutar BFS para mapear la zona afectada.
        analyze_connectivity_bfs(data, infected_seeds[i]);
    }

    free(infected_seeds);
    printf("[Tarea 2] Brotes iniciales y análisis de focos completados.\n");
}


// -------------------------------------------------------------
// --- (Punto 3): SIMULACIÓN DIARIA (O(N*K)) ---
// -------------------------------------------------------------

void run_daily_simulation(BIO_SIM_DATA *data, int dia_simulacion) {
    printf("\n[Tarea 3] Ejecutando Simulación Diaria (Día %d)...\n", dia_simulacion);

    // Iterar sobre las Tablas Hash de Personas (O(N) total)
    for (int i = 0; i < PERSON_HASH_TABLE_SIZE; i++) {
        PERSON_NODE *person_node = data->persons_table->table[i];
        
        while (person_node != NULL) {
            PERSON *p = &person_node->data;
            
            if (p->status == INFECTED) {
                // Acceso O(1) al Virus para sus parámetros
                STRAIN *cepa = get_cepa_by_id(data, p->actualStrainID);
                if (cepa == NULL) {
                    person_node = person_node->next;
                    continue; 
                }

                // 1. PROPAGACIÓN PROBABILÍSTICA (O(K) por individuo)
                // TODO: Implementar la iteración sobre la lista de contactos de 'p' 
                // para la simulación de contagio.
                
                // 2. RECUPERACIÓN / MUERTE
                p->daysInfected++;
                
                if (p->daysInfected >= (int)cepa->recovery) { 
                    // Chequeo de Letalidad
                    if (((double)rand() / RAND_MAX) < cepa->caseFatalityRatio) {
                        p->status = DEATH;
                    } else {
                        p->status = IMMUNE; // Recuperación
                    }
                }
            }
            person_node = person_node->next;
        }
    }

    // 3. GUARDAR HISTORIAL (Punto 8)
    save_contagion_history(data, dia_simulacion);
    printf("[Tarea 3] Simulación completada para el día %d.\n", dia_simulacion);
}


// -------------------------------------------------------------
// --- (Punto 7): CLUSTERING DE CEPAS (Placeholder) ---
// -------------------------------------------------------------

void cluster_strains_by_name(BIO_SIM_DATA *data) {
    // Implementación de la construcción del Árbol Trie (O(N*L))
    (void)data;
    printf("[Tarea 7] Clustering inicializado. Construcción del Trie pendiente.\n");
}
