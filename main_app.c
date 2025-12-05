// General Libraries
#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>

// Classes (Data structures and Logic)
#include "Clases/Person.h"
#include "Clases/Regions.h"
#include "Clases/Virus.h"

// Data access (DAO_General.h provides the initial data reading)
#include "DAO_General.h"

// Drawing & Plotting
#include "OpenGLDrawing/interface.h"
#include "OpenGLDrawing/utils.h"

// Algorithms (Logic for the main purpose of the program)
#include "Algorithms.h"

// Data repository for centralized variables for all of the components
#include "dataRepository.h"

// Function prototypes
void init_simulation_data();

// Main entry point
int main(int argc, char *argv[]) {
    init_simulation_data();

    if (GlobalData == NULL) {
        fprintf(stderr, "[FATAL_ERROR] Could not load initial data: Exiting\n");
        return 1;
    }

    init(&argc, argv);
    return 0;
}


// Loads all data structures and initializes the simulation environment
void init_simulation_data() {
    // Load initial data from CSV files
    GlobalData = load_initial_data(
        "Data/strains.txt", 
        "Data/regions.txt", 
        "Data/population.txt", 
        "Data/contactlist.txt",
        "Data/conexiones_regiones"
    );

    if (GlobalData != NULL) {
        printf("[SIM_INIT] Simulator initialized: Population loaded\n");
        
        // TASK 2: Set initial outbreaks
        establish_initial_outbreak(GlobalData, 10, 1);

        // TASK 5: CRITICAL PATH
        // Test: Find path between person 1 and 50 (Adjust IDs based on your data)
        // Only run if the population exists
        if (GlobalData->max_individuos >= 50) {
            find_most_probable_path(GlobalData, 1, 50);
        }
        
        // TASK 7: Initialize strain clustering
        cluster_strains(GlobalData);
    }
}