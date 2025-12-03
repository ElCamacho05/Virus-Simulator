#ifndef INTERFACE_H
#define INTERFACE_H

#include "DAO_General.h"

#define WIDTH 750
#define HEIGHT 750

// Function declaration
void init(int *argc, char *argv[]);
void display();
void drawInfectionLine(PERSON *p1, PERSON *p2);
void drawRegions(BIO_SIM_DATA *data);
void idle();
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);

#endif