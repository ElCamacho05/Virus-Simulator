#include "dataRepository.h"

#include <stdlib.h>
#include <GL/glut.h>
#include "DAO_General.h"

BIO_SIM_DATA *GlobalData = NULL; 
int pause = 0;
int simulation_day = 0;
double secondsPerDay = 1;

int elapsedTime = 0;

void updateTime() {
    elapsedTime = glutGet(GLUT_ELAPSED_TIME);
}