// Self Library
#include "Algorithms.h"

// General Libraries
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "Clases/Person.h"
#include "Clases/Virus.h"
#include "Clases/Regions.h"
#include "../DAO_General.h"
#include "dataRepository.h"

// Defines for event types
#define EVENT_RECOVERY 1
#define EVENT_DEATH 2
#define EVENT_IMMUNITY_LOSS 3 // Event for loss of immunity
#define EVENT_END_ISOLATION 4
#define EVENT_END_VACCINE_EFFECT 5

// External function for strain mutation (defined in Virus.c)
extern STRAIN* mutate_strain(STRAIN *parent, int new_id);

// Basic hash function for integer keys
unsigned int hashFunction(int key, int size) {
    return (unsigned int)key % size;
}

/*
---------------------------------------
------ ENSURING DATA CONSISTENCY ------
---------------------------------------
*/
void ensureConsistency(BIO_SIM_DATA *data) {
    if (!data) return;

    if (data->deathCount < 0) data->deathCount = 0;

    if (data->infectedCount < 0) data->infectedCount = 0;

    if (data->isolatedCount < 0) data->isolatedCount = 0;

    if (data->activeStrainCount < 0) data->activeStrainCount = 0;
    
    if (!data->regions_table) return;

    for (int i = 0; i < REGION_HASH_TABLE_SIZE; i++) { 
        REGION_NODE *current = (REGION_NODE *)data->regions_table->table[i];
        while (current != NULL) {
            REGION *r = &current->data;
            
            if (r->infectedCount < 0) r->infectedCount = 0;
            if (r->deathCount < 0) r->deathCount = 0;
            if (r->populationCount < 0) r->populationCount = 0;
            
            current = current->next;
        }
    }
}

/* ----------------------------------------------------------------------
   -----------------------  QUEUE IMPLEMENTATION  -----------------------
   (Required for BFS)
   ---------------------------------------------------------------------- */

void enqueue(Queue*q, int id) {
    // Allocates memory for a new node and adds it to the rear of the queue
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
    // Removes the node from the front of the queue and returns its person ID
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
----------------------------------------------------------------------
*/

// Analyzes the connectivity of an outbreak zone using Breadth-First Search (BFS)
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
    
    printf("  [BFS Analysis]: Outbreak zone from ID %d covers %d individuals.\n", start_person_id, total_affected_in_zone);
    free(visited);
}

// Adds a person to the array of active spreaders and updates regional stats
void add_to_active_infected(BIO_SIM_DATA *data, int person_id, char strainName[]) {
    printf("[INFECTED] - Person %d is now infected!!!\n", person_id);
    
    // 1. Add to the active infected array
    if (data->activeInfectedIDs && data->infectedCount < data->max_individuos) {
        data->activeInfectedIDs[data->infectedCount++] = person_id;
    }

    // 2. Update regional statistics
    PERSON *person = get_person_by_id(data, person_id);
    if (person) {
        REGION *region = get_region_by_id(data, person->regionID);
        if (region) region->infectedCount++;
    }
}

// Removes a person from the active spreader array and updates regional stats
void remove_from_active_infected(BIO_SIM_DATA *data, int person_id) {
    PERSON *person = get_person_by_id(data, person_id);
    if (!person) return;

    // Reset infection specific person data
    person->actualStrainID = -1;
    person->daysInfected = 0;
    person->infectedBy = -1;

    // Update regional count
    REGION *region = get_region_by_id(data, person->regionID);
    if (region) region->infectedCount--;

    // Remove from the activeInfectedIDs array by swapping with the last element
    for (int i = 0; i < data->infectedCount; i++) {
        if (data->activeInfectedIDs[i] == person_id) {
            data->activeInfectedIDs[i] = data->activeInfectedIDs[data->infectedCount - 1];
            data->infectedCount--;
            return;
        }
    }
}

/* ----------------------------------------------------------------------
-----------------------         SORTING        -----------------------
---------------------------------------------------------------------- 
*/

/*
------------
-- PERSON --
------------
*/
// Merge step for Person array (sorted by risk)
void mergePerson(PERSON **arr, int l, int m, int r) {
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;

    // Create temporary arrays of POINTERS
    PERSON **L = (PERSON**)malloc(n1 * sizeof(PERSON*));
    PERSON **R = (PERSON**)malloc(n2 * sizeof(PERSON*));

    // Copy data to temporary arrays
    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    // Merge the temporary arrays back into arr[l..r]
    i = 0; 
    j = 0; 
    k = l;
    while (i < n1 && j < n2) {
        // SORTING CRITERION: Descending Risk (>= for highest risk first)
        if (L[i]->initialRisk >= R[j]->initialRisk) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    // Copy remaining elements of L[], if any
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    // Copy remaining elements of R[], if any
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }

    free(L);
    free(R);
}

// Recursive function for Merge Sort
void mergeSortPersonArrayRecursivo(PERSON **arr, int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;

        mergeSortPersonArrayRecursivo(arr, l, m);
        mergeSortPersonArrayRecursivo(arr, m + 1, r);

        mergePerson(arr, l, m, r);
    }
}

// Public function to sort the person array (pre-processing for Greedy algorithm)
void sortPersonArray(BIO_SIM_DATA *data) {
    if (!data || !data->personArray || data->max_individuos <= 0) return;
    
    mergeSortPersonArrayRecursivo(data->personArray, 0, data->max_individuos - 1);
}

/*
------------
-- STRAIN --
------------
*/

// === SWAP UTILITIES ===
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
// ===  HEAPSORT - Adapted for dynamic growth
// ======================================================================

// Auxiliary structure to manage the heap
typedef struct {
    STRAIN *array;
    int n; // Current size
} StrainHeap;

void init_strain_heap(StrainHeap *h, int space) {
    // Dynamic allocation based on the current count
    h->array = (STRAIN *)malloc(sizeof(STRAIN) * space);
    h->n = 0;
}

// Push using "Float Up" (Max-Heap by ID)
void push_strain_heap(StrainHeap *h, STRAIN dato) {
    int i = h->n;
    h->array[i] = dato;
    h->n++;

    // Float Up
    for (; i > 0; i = (i - 1) / 2) {
        // Ordering by ID (can be changed to 'beta' or 'lethality' if needed)
        if (h->array[i].beta > h->array[(i - 1) / 2].beta) {
            swap_strain(&h->array[i], &h->array[(i - 1) / 2]);
        } else {
            break;
        }
    }
}

// Pop using "Sink Down"
STRAIN pop_strain_heap(StrainHeap *h) {
    STRAIN temp = h->array[0];
    int i = 0;
    
    // Move the last element to the root
    h->array[0] = h->array[(h->n) - 1];
    h->n--;

    // Sink Down
    int hMayor;
    while (2 * i + 1 < h->n) { 
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        
        // Find the largest child
        if (right < h->n && h->array[right].id > h->array[left].id)
            hMayor = right;
        else
            hMayor = left;

        // Swap if needed
        if (h->array[hMayor].id > h->array[i].id) {
            swap_strain(&h->array[i], &h->array[hMayor]);
            i = hMayor;
        } else {
            break;
        }
    }
    return temp;
}

// --- MAIN FUNCTION: DYNAMIC HEAPSORT ---
void sortStrainArray(BIO_SIM_DATA *data) {
    if (!data || !data->cepas_hash_table) return;

    // 1. GET ACTUAL SIZE (Crucial to avoid errors)
    int total_strains = data->cepas_hash_table->count;
    
    if (total_strains == 0) {
        printf("[Heapsort] No registered strains found.\n");
        return;
    }

    printf("\n[Heapsort] Sorting %d detected strains (including mutations)...\n", total_strains);

    // 2. Initialize Heap with the exact required size
    StrainHeap h;
    init_strain_heap(&h, total_strains);
    if (!h.array) return; // Memory error

    // 3. Extract all strains from the Hash Table to the Heap
    for (int i = 0; i < VIRUS_HASH_TABLE_SIZE; i++) {
        STRAIN_NODE *current = data->cepas_hash_table->table[i];
        while (current != NULL) {
            push_strain_heap(&h, current->data);
            current = current->next;
        }
    }

    // 4. Extract sorted (Pop returns the largest -> Descending Order)
    printf("--- List of Strains Sorted by ID (Desc) ---\n");
    for (int i = 0; i < total_strains; i++) {
        STRAIN s = pop_strain_heap(&h);
        printf("  ID: %d | Name: %s | Beta: %.2f | Mutation: %.3f\n", 
               s.id, s.name, s.beta, s.mutationProb);
    }
    printf("--------------------------------------------\n");

    // 5. Free temporary memory
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

// Lomuto Partition adapted for REGION array (sorting by Region ID)
int partition_region(REGION *arr, int low, int high) {
    int pivot = arr[high].id; // Pivot: Region ID
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) {
        // Ascending Order (smaller ID first)
        if (arr[j].id < pivot) {
            i++;
            swap_region(&arr[i], &arr[j]);
        }
    }
    swap_region(&arr[i + 1], &arr[high]);
    return (i + 1);
}

// Recursive function for QuickSort
void quickSort_region_recursive(REGION *arr, int low, int high) {
    if (low < high) {
        int pi = partition_region(arr, low, high);
        quickSort_region_recursive(arr, low, pi - 1);
        quickSort_region_recursive(arr, pi + 1, high);
    }
}

// --- MAIN FUNCTION: DYNAMIC QUICKSORT ---
// Sorts a temporary array of regions by ID for display purposes
void sortRegionArray(BIO_SIM_DATA *data) {
    if (!data || !data->regions_table) return;

    // 1. Get current count
    int total_regions = data->regions_table->count;
    if (total_regions == 0) return;

    // 2. Create temporary linear array (Data Snapshot)
    REGION *region_array = (REGION *)malloc(sizeof(REGION) * total_regions);
    if (!region_array) return;

    // 3. Fill the array from the Hash Table
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

    printf("\n[Quicksort] Sorting %d regions by ID...\n", total_regions);

    // 4. Execute QuickSort
    quickSort_region_recursive(region_array, 0, total_regions - 1);

    // 5. Display Result
    for (int i = 0; i < total_regions; i++) {
        printf("  Region ID: %d | %s | Population: %d | Infected: %d\n", 
               region_array[i].id, region_array[i].name, 
               region_array[i].populationCount, region_array[i].infectedCount);
    }

    // 6. Free temporary memory
    free(region_array);
}

/* ----------------------------------------------------------------------
-----------------------    INITIAL OUTBREAK    -----------------------
----------------------------------------------------------------------
*/

// Establishes the initial infected individuals (patient zero)
void establish_initial_outbreak(BIO_SIM_DATA *data, int num_brotes, int cepa_id) {
    if (num_brotes >= data->max_individuos) return;

    // Initialization of structures if they don't exist (Heap, Active Infected Array)
    if (!data->eventQueue) {
        // Large capacity for future events
        data->eventQueue = createMinHeap(data->max_individuos * 4); 
        data->activeInfectedIDs = (int*)malloc(data->max_individuos * sizeof(int));
        data->infectedCount = 0;
        data->deathCount = 0;
    }

    STRAIN *virus = get_cepa_by_id(data, cepa_id);
    int count = 0;

    int pIdx = 0;
    printf("[OUTBREAK] Infecting %d patient zeros...\n", num_brotes);
    while (count < num_brotes) {
        // int random_id = (rand() % data->max_individuos) + 1;
        PERSON *p = get_person_by_id(data, pIdx);
        
        if (p && p->status == HEALTH) {
            // Set initial infection status
            p->status = INFECTED;
            p->actualStrainID = cepa_id;
            p->daysInfected = 0;

            add_to_active_infected(data, p->id, virus->name);

            // Calculate duration and determine event type (Recovery or Death)
            int duracion = (int)(virus->recovery * 100);
            if (duracion <= 0) duracion = 7;

            int type = EVENT_RECOVERY;
            if (((double)rand() / RAND_MAX) < virus->caseFatalityRatio) {
                type = EVENT_DEATH;
            }

            // Schedule the end-of-infection event
            insertMinHeap(data->eventQueue, p->id, (double)duracion, type);
            count++;
        }
        pIdx++;
    }
}

/*
----------------------------------------------------------------------
-----------------------    DAILY SIMULATION    -----------------------
----------------------------------------------------------------------
*/

// Runs the simulation logic for one day, processing events and handling propagation
void run_daily_simulation(BIO_SIM_DATA *data, int dia_actual) {
    ensureConsistency(data);
    
    // TEMPORARY: Ensure at least one random person gets infected to keep the simulation going if the active infected list clears out
    if (data) {
        int randomPersonId = (rand() % data->max_individuos);
        int randomStrainId = (rand() % data->activeStrainCount);

        PERSON *target = get_person_by_id(data, randomPersonId);
        STRAIN *virus = get_cepa_by_id(data, randomStrainId);

        if (!target || !virus) goto a;
        
        if (target->status == HEALTH) {
            target->status = INFECTED;
            target->actualStrainID = randomStrainId;
            target->daysInfected = 0;
            target->infectedBy = -1; 

            add_to_active_infected(data, target->id, virus->name);
            
            // Schedule event
            int dur = (int)(virus->recovery * 100); 
            if (dur < 2) dur = 5; 
            int type = (((double)rand() / RAND_MAX) < virus->caseFatalityRatio) ? EVENT_DEATH : EVENT_RECOVERY;

            insertMinHeap(data->eventQueue, target->id, (double)(dia_actual + dur), type);
        }
    }

    a: {}

    // === PART 1: PROCESS EVENTS (Recoveries, Deaths, Isolation End, Immunity Loss) ===
    while (!isHeapEmpty(data->eventQueue)) {
        // Check if the next event has occurred (event time <= current day)
        if (data->eventQueue->array[0].value > dia_actual) break;

        HeapNode event = extractMinHeap(data->eventQueue);
        PERSON *p = get_person_by_id(data, event.id);
        
        if (!p) continue;

        // --- CASE: IMMUNITY LOSS (Return to Susceptible) ---
        if (event.type == EVENT_IMMUNITY_LOSS) {
            if (p->status == IMMUNE) {
                p->status = HEALTH;
                // Full reset so they can get infected again
                p->actualStrainID = -1;
                p->infectedBy = -1;
                p->daysInfected = 0;
                printf("  [NORMALIZED] Person %d returns to susceptible status.\n", p->id);
            }
            continue;
        }
        
        // --- CASE: END OF ISOLATION ---
        if (event.type == EVENT_END_ISOLATION) {
            if (p->status == ISOLATED) {
                p->status = HEALTH; // Returns to healthy status
                data->isolatedCount--; // Global decrement
                printf("  [ISOLATION_END] Person %d is released and healthy.\n", p->id);
            }
            continue;
        }

        // --- CASE: END OF VACCINE EFFECT ---
        if (event.type == EVENT_END_VACCINE_EFFECT) {
            if (p->status == VACCINATED) {
                p->status = HEALTH; // Returns to healthy status
                p->initialRisk *= 2.0; // Risk returns to normal (0.5 * 2 = 1.0)
                // printf("  [VACCINE_END] Person %d loses protection.\n", p->id);
            }
            continue;
        }


        // --- CASE: END OF INFECTION (Recovery or Death) ---
        
        if (p->status == INFECTED) {
            REGION *region = get_region_by_id(data, p->regionID);
            
            if (event.type == EVENT_DEATH) {
                p->status = DEATH;
                printf("!!![DEATH] Person %d (%s) has died.\n", p->id, p->name);
                
                // Update counters
                if (region) {
                    if (region->infectedCount > 0) region->infectedCount--;
                    region->deathCount++;
                }
                data->deathCount++;

            } else { // EVENT_RECOTERY
                // RECOVERY
                p->status = IMMUNE;
                printf("  [RECOVERED] Person %d (%s) is now immune.\n", p->id, p->name);
                
                if (region) {
                    if (region->infectedCount > 0) region->infectedCount--;
                }

                // --- VIRUS VALUE RESET ---
                p->actualStrainID = -1;
                p->daysInfected = 0;
                p->infectedBy = -1;

                // Lower risk by 5%
                p->initialRisk = p->initialRisk*0.95;

                // --- SCHEDULE IMMUNITY LOSS ---
                // Random immunity duration between 20 and 50 days
                int dias_inmune = 20 + (rand() % 31);
                insertMinHeap(data->eventQueue, p->id, (double)(dia_actual + dias_inmune), EVENT_IMMUNITY_LOSS);
            }
            
            // Stop processing as an active spreader
            remove_from_active_infected(data, p->id);
        }
    }

    // === PART 2: PROPAGATION (Static Graph) ===
    int current_infected_count = data->infectedCount; 
    
    // Iterate over all active infected individuals
    for (int i = 0; i < current_infected_count; i++) {
        int spreader_id = data->activeInfectedIDs[i];
        PERSON *spreader = get_person_by_id(data, spreader_id);
        
        if (!spreader || spreader->status != INFECTED) continue;

        STRAIN *virus = get_cepa_by_id(data, spreader->actualStrainID);
        if (!virus) continue;

        // Iterate over contacts (using STATIC ARRAY)
        for (int j = 0; j < spreader->numContacts; j++) {
            if (spreader->contacts[j].contactID <= 0) continue;

            int targetID = spreader->contacts[j].contactID;
            double contactWeight = spreader->contacts[j].weight;
            PERSON *target = get_person_by_id(data, targetID);

            // Check if the target person is HEALTHY (susceptible)
            if (target && target->status == HEALTH) {
                
                // Calculate Regional Factor (Distance)
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
                    else factorRegional = 0.05; // Base probability for long-distance travel
                }
                double personRiskFactor = (1+target->initialRisk)/2.0;
                double prob = virus->beta * contactWeight * factorRegional * personRiskFactor;

                // Attempt to infect
                if (((double)rand() / RAND_MAX) < prob) {
                    
                    int finalStrainID = virus->id;
                    
                    // Mutation Check
                    if (((double)rand() / RAND_MAX) < virus->mutationProb) {
                        static int mutation_id_counter = 100; 
                        mutation_id_counter++;
                        STRAIN *mutant = mutate_strain(virus, mutation_id_counter);
                        
                        if (mutant) {
                            printf("  [MUTATED] Strain %s mutated to -> %s (ID: %d)\n", virus->name, mutant->name, mutant->id);
                            
                            data->activeStrainCount++;
                            insertStrainInHash(data->cepas_hash_table, mutant);
                            
                            // Update TRIE if it exists
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

                    // Update status of the newly infected person
                    target->status = INFECTED;
                    target->actualStrainID = finalStrainID;
                    target->daysInfected = 0;
                    target->infectedBy = spreader->id; 

                    add_to_active_infected(data, target->id, virus->name);
                    // drawInfectionLine(target, spreader);

                    // Schedule termination event (Recovery/Death)
                    int dur = (int)(virus->recovery * 100); 
                    if (dur < 2) dur = 5; 
                    int type = (((double)rand() / RAND_MAX) < virus->caseFatalityRatio) ? EVENT_DEATH : EVENT_RECOVERY;

                    insertMinHeap(data->eventQueue, target->id, (double)(dia_actual + dur), type);
                }
            }
        }
        spreader->daysInfected++; // Increment infected days for the spreader
    }

    printf("\n\n=== END OF DAY %d | Active Infected: %d | Fatalities: %d ===\n\n\n", 
           dia_actual, data->infectedCount, data->deathCount);
}

/*
----------------------------------------------------------------------
-----------------------    RISK MINIMIZING     -----------------------
----------------------------------------------------------------------
*/

// --- RISK MINIMIZATION (GREEDY) ---
// Complexity: O(N) execution (after O(N log N) pre-processing using Merge Sort)
void minimize_total_risk(BIO_SIM_DATA *data, double reduction_target_percentage, int mode) {
    if (!data || !data->personArray || data->max_individuos == 0) return;

    printf("\n[GREEDY] Executing Greedy Isolation/Vaccination algorithm...\n");

    // 1. Calculate Maximum Limit of People to Isolate/Vaccinate
    int maxIsolatedLimit = (int)(data->max_individuos * reduction_target_percentage);
    
    // 2. Control variables
    double currentEliminatedRisk = 0.0;
    int newlyIsolatedCount = 0;

    int isoCount = 0;

    // 3. GREEDY SELECTION
    // Iterate over the already sorted array from HIGHEST RISK to LOWEST
    for (int i = 0; i < data->max_individuos; i++) {
        PERSON *p = data->personArray[i];

        // --- CAPACITY CEILING CONDITION ---
        // Stop if the maximum number of people for the intervention is reached
        if (isoCount >= maxIsolatedLimit) break;

        // Skip people who are already out of circulation or vaccinated
        if (p->status == DEATH || p->status == ISOLATED || p->status == VACCINATED) {
            continue; 
        }

        // If the person was infected, remove them from the active spreader list
        if (p->status == INFECTED) {
            remove_from_active_infected(data, p->id);
        }

        if (mode == ISOLATED) { 
            // 3.1a Apply isolation
            p->status = ISOLATED;
            // Schedule the end of isolation (ex, in 20 days)
            insertMinHeap(data->eventQueue, p->id, (double)(simulation_day + 20), EVENT_END_ISOLATION);
        }
            
        if (mode == VACCINATED) { 
            // 3.1b Apply vaccine
            p->status = VACCINATED;
            // Schedule the end of vaccine effect (ex, in 50 days)
            insertMinHeap(data->eventQueue, p->id, (double)(simulation_day + 50), EVENT_END_VACCINE_EFFECT);
            p->initialRisk *= 0.5; // Drastically reduced risk
        }
        

        // 3.2 Update Global Counters
        currentEliminatedRisk += p->initialRisk;
        data->isolatedCount++; // Global intervention count
        isoCount++;
        newlyIsolatedCount++;

    }
    printf("%d\n", data->max_individuos);
    printf("%d\n", maxIsolatedLimit);

    printf("  - Isolation/Vaccination Limit: %d people (%.2f%%)\n", maxIsolatedLimit, reduction_target_percentage * 100);
    if (mode == ISOLATED)
        printf("  - Isolated %d new high-risk people.\n", newlyIsolatedCount);
    else if (mode == VACCINATED) 
        printf("  - Vaccinated %d new high-risk people.\n", newlyIsolatedCount);

    printf("  - Accumulated risk eliminated today: %.2f\n", currentEliminatedRisk);
}

/*
----------------------------------------------------------------------
-----------------------   HEAP IMPLEMENTATION  -----------------------
----------------------------------------------------------------------
*/

// Creates and initializes a MinHeap structure
MinHeap* createMinHeap(int capacity) {
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
    if (!heap) return NULL;
    heap->size = 0;
    heap->capacity = capacity;
    heap->array = (HeapNode*)malloc(capacity * sizeof(HeapNode));
    return heap;
}

// Swaps two heap nodes
void swap(HeapNode *a, HeapNode *b) {
    HeapNode temp = *a;
    *a = *b;
    *b = temp;
}

// Maintains the min-heap property by sinking down the element at idx
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

// Checks if the heap is empty
int isHeapEmpty(MinHeap* heap) {
    return !heap || heap->size == 0;
}

// Inserts a new element and ensures the min-heap property is maintained (floating up)
void insertMinHeap(MinHeap* heap, int id, double value, int type) {
    if (!heap) return;
    // Dynamic resizing if capacity is reached
    if (heap->size == heap->capacity) {
        heap->capacity *= 2;
        heap->array = realloc(heap->array, heap->capacity * sizeof(HeapNode));
        if (!heap->array) return; 
    }

    int i = heap->size++;
    heap->array[i].id = id;
    heap->array[i].value = value;
    heap->array[i].type = type;

    // Float Up
    while (i && heap->array[(i - 1) / 2].value > heap->array[i].value) {
        swap(&heap->array[i], &heap->array[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

// Extracts the minimum value (root) and restores the min-heap property
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

// Frees the memory allocated for the heap
void freeMinHeap(MinHeap* heap) {
    if(heap) {
        if(heap->array) free(heap->array);
        free(heap);
    }
}

// ======================================================================
// ===  MOST SUSCEPTIBLE PATH (DIJKSTRA MAX PROB)
// ======================================================================

// Auxiliary structure for path reconstruction in Dijkstra's
typedef struct{
    int parent_id;
    double probability;
    bool visited;
}PathNode;

// Finds the path with the highest cumulative contagion probability (using Dijkstra's on -log(P))
double find_most_probable_path(BIO_SIM_DATA *data, int start_id, int end_id){
    if (!data || !data->persons_table) return 0;

    // Buffer limit for person IDs
    int limit = data->max_individuos + 100; 

    // Arrays for Dijkstra's implementation
    double *min_cost =(double*)malloc(limit *sizeof(double));
    int *parent = (int*)malloc(limit *sizeof(int));
    bool *visited = (bool*)calloc(limit, sizeof(bool));

    // Initialization
    for(int i=0; i<limit; i++){
        min_cost[i]= INFINITY;
        parent[i]=-1;  
    }

    // Min-Heap for Dijkstra (Heap value = cost -log P)
    MinHeap *pq = createMinHeap(limit);

    // Configure starting node: Probability 1.0 -> -log(1.0) = 0.0 cost
    min_cost[start_id] = 0.0;
    insertMinHeap(pq, start_id, 0.0, 0); 

    printf("\n[DIJKSTRA] Finding highest risk path: ID %d -> ID %d\n", start_id, end_id);

    bool found = false;

    while (!isHeapEmpty(pq)) {
        HeapNode current = extractMinHeap(pq);
        int u_id = current.id;
        double u_cost = current.value;

        // Optimization: Destination reached
        if (u_id == end_id) {
            found = true;
            break;
        }

        // Lazy deletion: Skip if a better path was found previously
        if (u_cost > min_cost[u_id]) continue;
        
        visited[u_id] = true;

        PERSON *u = get_person_by_id(data, u_id);
        if (!u) continue;

        // Iterate over neighbors (Contacts)
        for (int i = 0; i < u->numContacts; i++) {
            int v_id = u->contacts[i].contactID;
            double weight = u->contacts[i].weight; // Contact probability (0 to 1)

            if (v_id <= 0 || v_id >= limit) continue;
            if (visited[v_id]) continue;

            // Calculate cost: w = -log(probability)
            // Use a very small value to avoid log(0)
            if (weight <= 0.0001) weight = 0.0001;
            double edge_cost = -log(weight); 

            if (min_cost[u_id] + edge_cost < min_cost[v_id]) {
                min_cost[v_id] = min_cost[u_id] + edge_cost;
                parent[v_id] = u_id;
                insertMinHeap(pq, v_id, min_cost[v_id], 0);
            }
        }
    }

    double final_prob = 0.0;

    if (found) {
        // Reconstruct path
        final_prob = exp(-min_cost[end_id]); // Inverse of -log
        printf("  [PATH_FOUND] Path found! Total Probability: %.6f (%.2f%%)\n", final_prob, final_prob * 100);
        
        // Print route (Backtracking)
        printf("  [PATH_ROUTE]: ");
        int curr = end_id;
        int path_stack[1000];
        int stack_idx = 0;
        
        while (curr != -1) {
            path_stack[stack_idx++] = curr;
            curr = parent[curr];
        }
        
        for (int i = stack_idx - 1; i >= 0; i--) {
            printf("%d", path_stack[i]);
            if (i > 0) printf(" -> ");
        }
        printf("\n");

    } else {
        printf("  [PATH_NOT_FOUND] No path exists between %d and %d.\n", start_id, end_id);
    }

    // Free memory
    free(min_cost);
    free(parent);
    free(visited);
    freeMinHeap(pq); // Frees the temporary heap structure

    return final_prob;
}



// -------------------------------------------------------------
// --- STRAIN CLUSTERING (TRIE) ---
// -------------------------------------------------------------
/*
--------------------------------                                                                                                                                                                                                                                                                                
            TRIE
--------------------------------
*/
// Global declaration for the Trie root
// The struct TrieNode and related functions (createNode, insert) are assumed to be defined/available in Virus.c/Virus.h
struct TrieNode *strain_trie_root = NULL;

// Builds a TRIE structure to classify strains based on their names
void cluster_strains(BIO_SIM_DATA *data) {
    if (!data || !data->cepas_hash_table) return;

    if(strain_trie_root == NULL){
        // Initializes the Trie root node
        strain_trie_root = createNode(); 
    }
    printf("\n[TRIE_BUILD] Constructing the TRIE tree for strain classification....\n");

    // Iterate over all strains in the Hash Table and insert them into the Trie
    for(int i=0; i<VIRUS_HASH_TABLE_SIZE; i++){
        STRAIN_NODE *strain_node = data->cepas_hash_table->table[i];
        while (strain_node != NULL) {
            STRAIN *s = &strain_node->data;
            // Insertion into the TRIE (the 'insert' function is assumed available)
            insert(strain_trie_root, s->name);
            strain_node = strain_node->next;
        }
    }

    printf("[TRIE_BUILD] TRIE construction completed.\n");
}