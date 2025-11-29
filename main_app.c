// General Libraries
#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>

// Classes
#include "Clases/Person.h"
#include "Clases/Regions.h"
#include "Clases/Virus.h"

// Data retrieve
#include "DAO_General.h"

// Drawing & Plotting
#include "OpenGLDrawing/interface.h"

// Algorithms
#include "Algorithms/Algorithms.h"

// Hash Arrays of data
int *popArr;
int *regArr;
int *virArr;

/*
STRUCTS AND TYPES DEFINITION
*/

// Regions
typedef struct region{
    int id;
    char name[20];
    int infected;
} Region;

typedef struct adjacentRegion{
    Region *region;
    double distance;
    struct adjacentRegion *nextAdjReg;
} AdjReg;

int pause = 0;

void init();
void my_display(void);
void my_keyboard(unsigned char key, int x, int y);
void idle_func(void);

int main(int argc, char *argv[]) {
    iglSetDisplayFunc(my_display);
    iglInit(&argc, argv, iglGetLogicalWidth(), iglGetLogicalHeight(), "Apocalypse Simulator");
    iglSetIdleFunc(idle_func);
    iglSetKeyboardFunc(my_keyboard);
    iglRun();
    return 0;
}

void init() {
    // Initialize population Hash Array
    int maxPopArr = (int) sqrt(MAX_POPULATION - 0);
    int pop[maxPopArr];
    popArr = pop;

    // Initialize Regions Hash Array
    int maxRegArr = (int) sqrt(MAX_REGIONS - 0);
    int reg[maxRegArr];
    popArr = reg;

    // Initialize Viruses Hash Array
    int maxVirArr = (int) sqrt(MAX_VIRUSES - 0);
    int vir[maxVirArr];
    popArr = vir;

    // Here comes the rest of the data initialization logic
    //...
    // recommended use of getHash from 
}

void my_display(void) {
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        glVertex2f(0.0f, (float)iglGetLogicalHeight());
        glVertex2f((float)iglGetLogicalWidth() * 0.5f, (float)iglGetLogicalHeight() * 0.5f);
        glVertex2f((float)iglGetLogicalWidth(), (float)iglGetLogicalHeight());
        glVertex2f((float)iglGetLogicalWidth() * 0.5f, (float)iglGetLogicalHeight() * 0.5f);
    glEnd();

    glColor3f(0.0f, 0.75f, 0.75f);
    iglDrawCircle(iglGetLogicalWidth()/2.0, iglGetLogicalHeight()/2.0, 50, 30);
}

void my_keyboard(unsigned char key, int x, int y) {
    if (key == 27) exit(0); // esc / close
    if (key == 'p' || key == 'P') {pause = !pause; }; // pause
}

void idle_func(void) {
}
