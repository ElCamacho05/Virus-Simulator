// General Libraries
#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>

// Classes (Data structures and Logic)
#include "Clases/Person.h"
#include "Clases/Regions.h"
#include "Clases/Virus.h"

// Data retrieve (DAO_General.h provides the initial data reading)
#include "DAO_General.h"

// Drawing & Plotting
// #include "OpenGLDrawing/interface.h"
#include "OpenGLDrawing/interface.h"
#include "OpenGLDrawing/utils.h"

// Algorithms (Logic for the main purpose of the program)
#include "Algorithms/Algorithms.h"

// Data repository for centralized variables for all of the components
#include "dataRepository.h"

// General data container
// GlobalData = NULL; 
// pause = 0;
// simulation_day = 0;

// Function prototypes
void init_simulation_data();

// Main entry point
int main(int argc, char *argv[]) {
    init_simulation_data();

    if (GlobalData == NULL) {
        fprintf(stderr, "Error fatal: No se pudieron cargar los datos iniciales. Finalizando.\n");
        return 1;
    }

    init(&argc, argv);

    free_biosim_data(GlobalData);
    return 0;
}


void init_simulation_data() {
    // Load initial data from CSV files
    GlobalData = load_initial_data(
        "Data/strains.txt", 
        "Data/regions.txt", 
        "Data/population.txt", 
        "Data/contactlist.txt"
    );

    if (GlobalData != NULL) {
        printf("Simulator initialized. Population loaded.\n");
        
        // TODO: Set initial outbreaks (Task 2)
        // establish_initial_outbreak(GlobalData, 10, 1);
        
        // TODO: Initialize strain clustering (Task 7)
        // cluster_strains(GlobalData);
    }
}


