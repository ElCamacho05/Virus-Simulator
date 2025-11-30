#ifndef INTERFACE_H
#define INTERFACE_H

#define WIDTH 500
#define HEIGHT 500

// Function declaration
void init(int *argc, char *argv[]);
void display();
void idle();
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);

#endif