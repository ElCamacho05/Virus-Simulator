#ifndef ALGORITHMS_H
#define ALGORITHMS_H

// #include "../DAO_General.h"
typedef struct BioSimData BIO_SIM_DATA;

unsigned int hashFunction(int key, int size);

// ----------------------------------------------------------------------
// ---------                AUX ALGORITHMS                      ---------
// ----------------------------------------------------------------------
void analyze_connectivity_bfs(BIO_SIM_DATA *data, int start_person_id);
void add_to_active_infected(BIO_SIM_DATA *data, int person_id);
void remove_from_active_infected(BIO_SIM_DATA *data, int person_id);

//PUNTO 2 (brotes aleatorios)
/*
--------------------------------
        INITIAL OUTBREAK
--------------------------------
*/
void establish_initial_outbreak(BIO_SIM_DATA *data, int num_brotes, int cepa_id);

//PUNTO 3(Simulacion de los contagios diarios)
/*
--------------------------------
        DAILY SIMULATION
--------------------------------
*/
void run_daily_simulation(BIO_SIM_DATA *data, int dia_simulation);

// PUNTO 7 Agrupar cepas por estructuras de nombre (O(L) con Trie)
void cluster_strains(BIO_SIM_DATA *data);

// Algoritmo de Grafo auxiliar (para Tarea 2)
void run_bfs_for_detection(BIO_SIM_DATA *data, int initial_person_id);


//Estructura de cola para BFS
/*
--------------------------------
            QUEUE
--------------------------------
*/
typedef struct QueueNode{
    int person_id;
    struct QueueNode *next;
}QueueNode;

typedef struct{
    QueueNode *front;
    QueueNode *rear;
}Queue;

void enqueue(Queue*q, int id);
int dequeue(Queue *q);

/*
--------------------------------
             HEAP
--------------------------------
*/
typedef struct {
    int id;          // Node ID / Person ID
    double value; // Day of recovery or Cost (for Dijkstra)
    int type; // 1: recovery / 2: death...
} HeapNode;

// Estructura del Min-Heap
typedef struct {
    HeapNode *array;
    int capacity;
    int size;
} MinHeap;

MinHeap* createMinHeap(int capacity);
void insertMinHeap(MinHeap* heap, int id, double value, int type);
HeapNode extractMinHeap(MinHeap* heap);
int isHeapEmpty(MinHeap* heap);
void freeMinHeap(MinHeap* heap);

#endif