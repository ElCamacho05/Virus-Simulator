/*
 * interface.c
 * Implementación modular para inicializar una ventana GLUT y permitir que
 * otros ficheros C registren callbacks para display/reshape/keyboard/idle.
 */

#include <stdio.h>
#include <GL/glut.h>
#include <math.h>
#include "interface.h"

#define PI 3.1415926535898

/* Tamaño lógico por defecto (puede ser cambiado en iglInit) */
static int logical_width = 500;
static int logical_height = 500;

/* Callbacks de usuario (pueden ser NULL) */
static IGLDisplayFunc user_display = NULL;
static IGLReshapeFunc user_reshape = NULL;
static IGLKeyboardFunc user_keyboard = NULL;
static IGLIdleFunc user_idle = NULL;

/* Utilidad: dibujar círculo (se mantiene disponible internamente) */

void circle(float x, float y, float rad, int n_seg) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y); // centro
    for (int i = 0; i <= n_seg; i++) {
        float ang = 2 * PI / n_seg * i;
        glVertex2f(x + rad * cos(ang), y + rad * sin(ang)); // punto de la circunferencia
    }
    glEnd();
}

/* Comportamiento por defecto de display: una línea guía desde (0,0) hasta el centro
 * del sistema de coordenadas lógico. Si el usuario registra su propio display,
 * ese se usará en su lugar.
 */
static void display_wrapper(void) {
    glClear(GL_COLOR_BUFFER_BIT);

    if (user_display) {
        user_display();
    } else {
        glColor3f(0.0f, 1.0f, 0.0f);
        glLineWidth(4.0f);
        glBegin(GL_LINES);
            glVertex3f(0.0f, 0.0f, 0.0f);
            glVertex3f((float)logical_width * 0.5f, (float)logical_height * 0.5f, 0.0f);
        glEnd();
    }

    glFlush();
    glutSwapBuffers();
}

/* Wrappers para reshape/keyboard/idle que aplican la proyección deseada y
 * llaman a los callbacks del usuario si existen.
 */
static void reshape_wrapper(int w, int h) {
    if (h == 0) h = 1;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    /* Estrategia: usamos un sistema de coordenadas lógico fijo. Esto fuerza
     * que el contenido se estire/contraiga cuando la ventana cambie de tamaño
     * porque el viewport (w,h) mapeará las coordenadas lógicas al tamaño real.
     */
    gluOrtho2D(0.0, (GLdouble)logical_width, 0.0, (GLdouble)logical_height);

    glMatrixMode(GL_MODELVIEW);

    if (user_reshape) user_reshape(w, h);
}

static void keyboard_wrapper(unsigned char key, int x, int y) {
    if (user_keyboard) user_keyboard(key, x, y);
}

static void idle_wrapper(void) {
    if (user_idle) user_idle();
    else glutPostRedisplay();
}

/* API pública */
void iglInit(int *argc, char **argv, int logical_w, int logical_h, const char *title) {
    if (logical_w > 0) logical_width = logical_w;
    if (logical_h > 0) logical_height = logical_h;

    glutInit(argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(logical_width, logical_height);
    glutInitWindowPosition(300, 200);
    glutCreateWindow(title ? title : "GL Window");

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glShadeModel(GL_FLAT);

    /* Registrar wrappers */
    glutDisplayFunc(display_wrapper);
    glutReshapeFunc(reshape_wrapper);
    glutKeyboardFunc(keyboard_wrapper);
    glutIdleFunc(idle_wrapper);
}

void iglSetDisplayFunc(IGLDisplayFunc f) { user_display = f; }
void iglSetReshapeFunc(IGLReshapeFunc f) { user_reshape = f; }
void iglSetKeyboardFunc(IGLKeyboardFunc f) { user_keyboard = f; }
void iglSetIdleFunc(IGLIdleFunc f) { user_idle = f; }

int iglGetLogicalWidth(void) { return logical_width; }
int iglGetLogicalHeight(void) { return logical_height; }

void iglRun(void) { glutMainLoop(); }

/* Exported wrapper to draw a circle using the internal utility. This keeps
 * the public API stable (iglDrawCircle) while the internal helper is
 * named `circle`.
 */
void iglDrawCircle(float x, float y, float rad, int n_seg) {
    circle(x, y, rad, n_seg);
}

/* Testing / utility helpers implementations */
void iglSetLogicalSize(int w, int h) {
    if (w > 0) logical_width = w;
    if (h > 0) logical_height = h;
}

int iglHasDisplayFunc(void) { return user_display != NULL; }
int iglHasReshapeFunc(void) { return user_reshape != NULL; }
int iglHasKeyboardFunc(void) { return user_keyboard != NULL; }
int iglHasIdleFunc(void) { return user_idle != NULL; }

