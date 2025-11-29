// General Libraries
#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>

// Clases (Estructuras de datos y lógica O(1))
#include "Clases/Person.h"
#include "Clases/Regions.h"
#include "Clases/Virus.h"

// Data retrieve (DAO_General.h provee la función load_initial_data)
#include "DAO_General.h"

// Drawing & Plotting
#include "OpenGLDrawing/interface.h"

// Algorithms (Se incluirá la lógica de la Tarea 2, 3, etc.)
#include "Algorithms/Algorithms.h"

// --- VARIABLES GLOBALES DEL SIMULADOR ---
// El objeto BioSimData es el contenedor central de todos los datos (el DAO).
BioSimData *GlobalData = NULL; 
int pause = 0;
int simulation_day = 0;

/*
STRUCTS AND TYPES DEFINITION
(SE ELIMINAN LAS DEFINICIONES DUPLICADAS DE REGION Y ADJREGION.
DEBEN ESTAR SOLO EN Clases/Regions.h)
*/

// Prototipos de funciones
void init_simulation_data(); // Función de inicialización de datos (reemplaza init())
void my_display(void);
void my_keyboard(unsigned char key, int x, int y);
void idle_func(void);

// --- FUNCIÓN PRINCIPAL ---
int main(int argc, char *argv[]) {
    // 1. CARGA DE DATOS Y ESTABLECIMIENTO DEL DAO (Tarea 1 de Armando)
    init_simulation_data();

    if (GlobalData == NULL) {
        fprintf(stderr, "Error fatal: No se pudieron cargar los datos iniciales. Finalizando.\n");
        return 1;
    }
    
    // 2. INICIALIZACIÓN GRÁFICA Y BUCLE PRINCIPAL
    iglSetDisplayFunc(my_display);
    iglInit(&argc, argv, iglGetLogicalWidth(), iglGetLogicalHeight(), "Apocalypse Simulator");
    iglSetIdleFunc(idle_func);
    iglSetKeyboardFunc(my_keyboard);
    iglRun();
    
    // Asegurar la liberación de memoria si el bucle iglRun termina
    free_biosim_data(GlobalData); 
    return 0;
}

// --- LÓGICA DE INICIALIZACIÓN DE DATOS (TAREA 1) ---
void init_simulation_data() {
    // 1. Llamada al DAO para cargar datos O(1)
    // NOTA: Los nombres de archivo deben coincidir con los creados manualmente
    GlobalData = load_initial_data(
        "cepas.txt", 
        "territorios.txt", 
        "individuos.txt", 
        "contactos.txt"
    );

    if (GlobalData != NULL) {
        printf("Simulador Inicializado. Población cargada.\n");
        
        // 2. Establecer Brotes Iniciales (Tarea 2, se llama a la función aquí)
        // Ejemplo: 10 brotes con la Cepa ID 1
        // establish_initial_outbreak(GlobalData, 10, 1);
        
        // 3. Inicializar el Clustering de Cepas (Tarea 7)
        // cluster_strains(GlobalData);
    }

    // ELIMINADO: La lógica de inicialización de popArr, regArr, virArr que usaba
    // memoria de stack (int pop[maxPopArr];) es incorrecta y se reemplaza por el DAO.
}


// --- FUNCIONES DE GRÁFICOS Y BUCLE ---

void my_display(void) {
    // ELIMINADA la lógica de dibujo simple que no refleja el estado del simulador
    
    // Lógica para dibujar la visualización del simulador usando GlobalData
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glColor3f(1.0f, 1.0f, 1.0f); 
    // ... (Tu lógica OpenGL para dibujar regiones/personas) ...
    glutSwapBuffers();
}

void my_keyboard(unsigned char key, int x, int y) {
    if (key == 27) { 
        free_biosim_data(GlobalData); // Liberar memoria antes de salir
        exit(0); 
    }
    if (key == 'p' || key == 'P') {pause = !pause; }; // pause
}

void idle_func(void) {
    if (!pause && GlobalData) {
        // Ejecutar el ciclo de simulación si no está en pausa
        simulation_day++;
        
        // Aquí se llamaría a la función de Propagación (Tarea 3 de Armando)
        // run_daily_simulation(GlobalData, simulation_day); 

        // Repintar para mostrar cambios
        glutPostRedisplay(); 
    }
}