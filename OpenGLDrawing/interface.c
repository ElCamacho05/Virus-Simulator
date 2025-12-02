#include "interface.h"
#include <GL/glut.h>
#include <stdio.h>

#include "utils.h"
#include "Algorithms.h"
#include "DAO_General.h"
#include "dataRepository.h"
#include "Regions.h"
#include "Person.h"

#define X 0
#define Y 1

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
    // const GLubyte* vendor = glGetString(GL_VENDOR);
    // const GLubyte* renderer = glGetString(GL_RENDERER);
    // const GLubyte* version = glGetString(GL_VERSION);
    // if (vendor && renderer && version) {
    //     printf("VENDOR: %s\n", vendor);
    //     printf("RENDERER: %s\n", renderer);
    //     printf("VERSION: %s\n", version);
    // }
    

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
    
    drawRegions(GlobalData);

    glutSwapBuffers();
}


void drawRegions(BIO_SIM_DATA *data) {
    REGION_HASH_TABLE *reg = data->regions_table;
    PERSON_HASH_TABLE *pop = data->persons_table;
    // Draw population
    float r = 0.0, g = 0.0, b = 0.0, alpha = 1.0;
    for (int i = 0; i< PERSON_HASH_TABLE_SIZE; i++) {
        PERSON_NODE *pN = pop->table[i];
        while (pN) {
            if (pN->data.status == HEALTH) {
                r = 0.0; g = 1.0, b = 0.0;
            }
            else if(pN->data.status == INFECTED) {
                r = 1.0; g = 0.0; b = 0.0;
            }
            else if(pN->data.status == IMMUNE) {
                r = 0.0; g = 0.0; b = 1.0;
            }
            else if(pN->data.status == DEATH) {
                glColor3f(0.0, 0.0, 0.0);
            }
            glPushMatrix();
                glTranslatef(pN->data.drawConf.pos[X], pN->data.drawConf.pos[Y], 0.0);
                circle(3.0, 36, r, g, b, alpha);
            glPopMatrix();
            pN = pN->next;
        }
    }
    // Draw regions
    for (int i = 0; i< REGION_HASH_TABLE_SIZE; i++) {
        REGION_NODE *rN = reg->table[i];
        while (rN) {
            glPushMatrix();
                glTranslatef(rN->data.drawConf.pos[X], rN->data.drawConf.pos[Y], 0.0);
                circle(rN->data.drawConf.radio, 36, 1.0, 1.0, 1.0, 1.0);
            glPopMatrix();
            rN = rN->next;
        }
    }

    
}

void idle() {
    if (GlobalData && !pause) {
        updateTime();
        int actualDay = (int)(elapsedTime/1000/secondsPerDay);
        if (actualDay > simulation_day) {
            simulation_day++;
            run_daily_simulation(GlobalData, simulation_day); 
        }
        
    }
    glutPostRedisplay(); // Solicita el redibujo para actualizar la visualización
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