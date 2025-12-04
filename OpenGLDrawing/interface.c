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

void drawInfectionLine(PERSON *p1, PERSON *p2) {
    glColor3f(1.0, 0.0, 0.0);
    glLineWidth(1.0);
    glBegin(GL_LINES);
        glVertex3f(p1->drawConf.pos[X], p1->drawConf.pos[Y], 0.0);
        glVertex3f(p2->drawConf.pos[X], p2->drawConf.pos[Y], 0.0);
    glEnd();
}

void drawRegions(BIO_SIM_DATA *data) {
    REGION_HASH_TABLE *reg = data->regions_table;
    PERSON_HASH_TABLE *pop = data->persons_table;
    // Draw population
    float r = 0.0, g = 0.0, b = 0.0, alpha = 1.0;
    for (int i = 0; i< PERSON_HASH_TABLE_SIZE; i++) {
        PERSON_NODE *pN = pop->table[i];
        while (pN) {
            PERSON *p = get_person_by_id(data, pN->data.id);
            if (p->infectedBy != -1 && p->status != DEATH)
                drawInfectionLine(p, get_person_by_id(data, p->infectedBy));
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
                r = 0.0; g = 0.0; b = 0.0;
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
            double infectionRatio = 0.0;
            if (rN->data.populationCount > 0) {
                infectionRatio = (double)rN->data.infectedCount / (double)rN->data.populationCount;
            }

            if (infectionRatio > 1.0) infectionRatio = 1.0;

            double r = 1.0;
            double g = 1.0 - infectionRatio; // Si ratio es 1.0 (100% infectados), g = 0
            double b = 1.0 - infectionRatio; // Si ratio es 1.0, b = 0
            
            double alpha = 0.3 + (infectionRatio * 0.5);
            
            glPushMatrix();
                glTranslatef(rN->data.drawConf.pos[X], rN->data.drawConf.pos[Y], 0.0);
                // circle(rN->data.drawConf.radio, 36, 1.0, 1.0, 1.0, 1.0);
                glColor4f(r, g, b, alpha);
                circle(rN->data.drawConf.radio, 36, r, g, b, alpha);
            glPopMatrix();
            rN = rN->next;
        }
    }
}

void drawHistorialPanel(BIO_SIM_DATA *data) {
    HISTORY_HASH_TABLE *history = data->history_table;

    
}

void idle() {
    if (GlobalData && !pause) {
        updateTime();
        int actualDay = (int)(elapsedTime/1000/secondsPerDay);
        if (actualDay > simulation_day) {
            printf("Simulando dia %d.\n",actualDay);
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

void keyboard(unsigned char key, int x, int y) {
    if (key == 27)
        exit(0);
}