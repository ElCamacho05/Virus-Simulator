#include "dataRepository.h"

#include <stdlib.h>
#include <GL/glut.h>
#include "DAO_General.h"

// Global pointer to the main simulation data structure
BIO_SIM_DATA *GlobalData = NULL; 
// Flag to pause the simulation logic
int pause = 0;
// Current simulated day number
int simulation_day = 0;
// Factor: how many real seconds equal one simulated day
double secondsPerDay = 2;

// Stores the time elapsed since the simulation started (in ms)
int elapsedTime = 0;

// Fetches the current elapsed time from the GLUT environment
void updateTime() {
    elapsedTime = glutGet(GLUT_ELAPSED_TIME);
}