#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include "interface.h"


/*
STRUCTS AND TYPES DECLARATION
*/

// regions
typedef struct region Region;
typedef struct adjacentRegion AdjReg;
// people
typedef struct person Person;
typedef struct contactlist ContList;

// Virus & Strains
typedef struct virus Virus;
typedef struct strain Strain;


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

// People
typedef struct person{
    int id;
    char name[30];
    Region *region;
    double initialDegree;
    double contagiousness;
    ContList *directory;
    int daysInfected;
} Person;

typedef struct contactlist{
    Person *person;
    double contagiousnessProbability;
    struct contactlist *next;
} ContList;

// Viruses
typedef struct virus{
    int id;
    char name[20];
} Virus;

typedef struct strain{
    Virus *father;
    double beta; // ??
    double caseFatalityRatio;
    double recovery; // ??
} Strain;




int pause = 0;

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
    // printf("Pause: %d\n", pause);
}

int main(int argc, char *argv[]) {
    iglSetDisplayFunc(my_display);
    iglInit(&argc, argv, iglGetLogicalWidth(), iglGetLogicalHeight(), "Apocalypse Simulator");
    iglSetIdleFunc(idle_func);
    iglSetKeyboardFunc(my_keyboard);
    iglRun();
    return 0;
}