/* example_app.c
 * Example usage of the interface.c library through interface.h.
 */

#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include "interface.h"

/* Display callback: draw guide lines and circle at center */
void my_display(void) {
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        glVertex2f(-iglGetLogicalWidth()/2.0, iglGetLogicalHeight()/2.0);
        glVertex2f(iglGetLogicalWidth()/2.0 * 0.5f, iglGetLogicalHeight()/2.0 * 0.5f);
        glVertex2f(iglGetLogicalWidth()/2.0, iglGetLogicalHeight()/2.0);
        glVertex2f(iglGetLogicalWidth()/2.0 * 0.5f, iglGetLogicalHeight()/2.0 * 0.5f);
    glEnd();

    glColor3f(0.0f, 0.75f, 0.75f);
    iglDrawCircle(0.0, 0.0, 50, 30, 1.0);
}

void my_keyboard(unsigned char key, int x, int y) {
    if (key == 27) exit(0);
}

void idleFunc(void) {
}

int main(int argc, char **argv) {
    iglInit(&argc, argv, 500, 500, "Example IGL");
    iglSetDisplayFunc(my_display);
    iglSetKeyboardFunc(my_keyboard);
    iglSetIdleFunc(idleFunc);
    iglRun();
    return 0;
}
