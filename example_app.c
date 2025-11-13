/* example_app.c
 * Ejemplo de uso de la librería `interface.c` a través de `interface.h`.
 */

#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include "interface.h"



/* Callback de display: dibuja una línea guía desde la esquina inferior izquierda
 * hasta el centro del sistema de coordenadas lógico.
 */
void my_display(void) {
    // Línea guía
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
    if (key == 27) exit(0); // esc
}

void idleFunc(void) {
    printf("");
};

int main(int argc, char **argv) {
    iglInit(&argc, argv, 500, 500, "Example IGL");
    iglSetDisplayFunc(my_display);
    iglSetKeyboardFunc(my_keyboard);
    iglSetIdleFunc(idleFunc);
    iglRun();
    return 0;
}
