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
#include "Algorithms.h"

// Data repository for centralized variables for all of the components
#include "dataRepository.h"

// General data container
// GlobalData = NULL; 
// pause = 0;
// simulation_day = 0;

// Function prototypes
void init_simulation_data();
void my_display(void);
void my_keyboard(unsigned char key, int x, int y);
void idle_func(void); // CLAVE: Controla la simulación diaria

// Main entry point
int main(int argc, char *argv[]) {
    init_simulation_data();

    if (GlobalData == NULL) {
        fprintf(stderr, "Error fatal: No se pudieron cargar los datos iniciales. Finalizando.\n");
        return 1;
    }

    init(&argc, argv);

    // 3. Registro de Callbacks
    glutDisplayFunc(my_display);
    glutIdleFunc(idle_func); // Para el avance de la simulación (Tarea 3)
    glutKeyboardFunc(my_keyboard); // Para salir/pausar

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
        establish_initial_outbreak(GlobalData, 10, 1);
        
        // TODO: Initialize strain clustering (Task 7)
        //cluster_strains(GlobalData);
    }
}

void my_display(void) {
    // Aquí iría la lógica para dibujar el estado actual del simulador
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // ... (Lógica de dibujo de regiones, personas infectadas/sanas) ...
    glutSwapBuffers();
}

void my_keyboard(unsigned char key, int x, int y) {
    // Se asume que GlobalData->pause y GlobalData->simulation_day existen via dataRepository.h
    
    if (key == 27) { // ESC
        free_biosim_data(GlobalData); 
        exit(0); 
    }
    if (key == 'p' || key == 'P') {
        // Alternar el estado de pausa (asumiendo que 'pause' existe en dataRepository)
        // pause = !pause; 
    }; 
}

void idle_func(void) {
    // Esta función controla el avance del tiempo de la simulación
    // if (!pause && GlobalData) {
    if (GlobalData) {
        // Tarea 3: Simulación probabilística diaria
        simulation_day++;
        run_daily_simulation(GlobalData, simulation_day); 
        
        glutPostRedisplay(); // Solicita el redibujo para actualizar la visualización
    }
}
