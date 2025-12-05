#ifndef ALGORITHMS_H
#define ALGORITHMS_H

// Forward declaration for the main data structure
typedef struct BioSimData BIO_SIM_DATA;

// Basic hash function prototype
unsigned int hashFunction(int key, int size);

// ----------------------------------------------------------------------
// ---------                AUX ALGORITHMS                      ---------
// ----------------------------------------------------------------------
void analyze_connectivity_bfs(BIO_SIM_DATA *data, int start_person_id);
void add_to_active_infected(BIO_SIM_DATA *data, int person_id, char strainName[]);
void remove_from_active_infected(BIO_SIM_DATA *data, int person_id);

/* ----------------------------------------------------------------------
-----------------------         SORTING        -----------------------
---------------------------------------------------------------------- 
*/

void sortPersonArray(BIO_SIM_DATA *data); // Sorts Persons by Risk (Mergesort)
void sortStrainArray(BIO_SIM_DATA *data); // Sorts Strains by Beta Value (Heapsort)
void sortRegionArray(BIO_SIM_DATA *data); // Sorts Regions by Population count (Quicksort)

/*
--------------------------------
        INITIAL OUTBREAK
--------------------------------
*/
// Initializes the simulation by establishing the first infected individuals (patient zeros)
void establish_initial_outbreak(BIO_SIM_DATA *data, int num_brotes, int cepa_id);

/*
--------------------------------
        DAILY SIMULATION
--------------------------------
*/
// Runs the simulation logic for one day, processing events and handling propagation
void run_daily_simulation(BIO_SIM_DATA *data, int dia_simulation);

/*
----------------------------------------------------------------------
-----------------------    RISK MINIMIZING     -----------------------
----------------------------------------------------------------------
*/
// Applies the Greedy algorithm (Isolation/Vaccination) to mitigate total risk
void minimize_total_risk(BIO_SIM_DATA *data, double reduction_target_percentage, int mode);

// Groups strains by name structure using a Trie tree (O(L) search time)
void cluster_strains(BIO_SIM_DATA *data);

// Auxiliary Graph Algorithm (for Task 2)
void run_bfs_for_detection(BIO_SIM_DATA *data, int initial_person_id);

// Finds the path with the highest probability of infection using Dijkstra's algorithm
double find_most_probable_path(BIO_SIM_DATA *data, int start_id, int end_id);


/*
--------------------------------
            QUEUE
--------------------------------
*/
// Structure for a single node in the queue (used for BFS)
typedef struct QueueNode{
    int person_id;
    struct QueueNode *next;
}QueueNode;

// Structure for the Queue head and tail
typedef struct{
    QueueNode *front;
    QueueNode *rear;
}Queue;

// Prototypes for Queue operations
void enqueue(Queue*q, int id);
int dequeue(Queue *q);

/*
--------------------------------
             HEAP
--------------------------------
*/
// Structure for a node in the Min-Heap
typedef struct {
    int id;          // Node ID / Person ID
    double value; // Day of recovery/event or Cost (for Dijkstra)
    int type; // 1: recovery / 2: death...
} HeapNode;

// Structure for the Min-Heap implementation (Event Queue for simulation)
typedef struct {
    HeapNode *array;
    int capacity;
    int size;
} MinHeap;

// Prototypes for Min-Heap operations
MinHeap* createMinHeap(int capacity);
void insertMinHeap(MinHeap* heap, int id, double value, int type);
HeapNode extractMinHeap(MinHeap* heap);
int isHeapEmpty(MinHeap* heap);
void freeMinHeap(MinHeap* heap);

#endif