#include <GL/glut.h>

double PI =  3.1415926535898;


void text(char *texto, float x, float y) {
    
    float l = glutBitmapLength(GLUT_BITMAP_HELVETICA_12, texto);
    glRasterPos2f(x-(l/2.0), y);

    for (int i = 0; i < strlen(texto); i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, texto[i]);
    }
}

void circle(float rad, int n_seg,double r, double g, double b, double alpha) {
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(r, g, b, alpha);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0, 0.0);
    for (int i = 0; i <= n_seg; i++) {
        float ang = 2 * PI / n_seg * i;
        glVertex2f(rad * cos(ang),rad * sin(ang));
    }
    glEnd();

    // glDisable(GL_BLEND);
}