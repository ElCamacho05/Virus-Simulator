#include "interface.h"
#include <GL/glut.h>
#include <stdio.h>
#include "utils.h"

// Function definition


void init(int *argc, char *argv[]) {
    glutInit(argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Virus Simulator V2");

    /* Disable lighting for 2D rendering */
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_NORMALIZE);
    glShadeModel(GL_FLAT);

    // glClearColor(0.0, 0.0, 0.0, 1.0); // black
    // glClearColor(1.0f, 1.0f, 1.0f, 0.0f); // white
    glClearColor(0.2, 0.2, 0.4, 1.0); // navy blue

    /* Graphics info */
    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    printf("VENDOR: %s\n", vendor);
    printf("RENDERER: %s\n", renderer);
    printf("VERSION: %s\n", version);

    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    glutMainLoop();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Draw test circle at origin */
    glColor3f(1.0, 1.0, 1.0);
    circle(50.0, 30, 1.0, 1.0, 1.0, 1.0);
    
    /* Draw guide lines from origin */
    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_LINES);
        glVertex2d(0.0, 0.0);
        glVertex2d(125, 0.0);
        glVertex2d(0.0, 0.0);
        glVertex2d(0.0, 125);
    glEnd();

    glutSwapBuffers();
}



 void reshape(int w, int h)
{
    if (h == 0) h = 1;
    
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    /* Maintain aspect ratio: calculate logical dimensions proportionally */
    float aspect = (float)w / (float)h;
    float logical_h = 500.0;  /* Base height */
    float logical_w = logical_h * aspect;  /* Width scales with aspect ratio */
    
    /* Orthographic projection centered at origin */
    gluOrtho2D(-logical_w/2.0, logical_w/2.0, -logical_h/2.0, logical_h/2.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


// void reshape(int w, int h)
// {
// 	glViewport (0, 0, (GLsizei) w, (GLsizei) h); // establecer la vista a todo
//    	glMatrixMode (GL_PROJECTION); // usar la matriz de proyección
//    	glLoadIdentity (); // y cargarla
//    	// establecer a toda la pantalla
//    	// gluOrtho2D (0.0, (GLdouble) w, 0.0, (GLdouble) h);
//     // glMatrixMode(GL_MODELVIEW);

// }

void keyboard(unsigned char key, int x, int y) {
    if (key == 27)
        exit(0);
}