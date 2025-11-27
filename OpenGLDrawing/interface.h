/* interface.h
 * Pequeña API para inicializar una ventana GLUT y registrar callbacks.
 * Diseñada para incluir desde otros archivos C y reutilizar la lógica de ventana.
 */
#ifndef IGL_INTERFACE_H
#define IGL_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Tipos de callback */
typedef void (*IGLDisplayFunc)(void);
typedef void (*IGLReshapeFunc)(int width, int height);
typedef void (*IGLKeyboardFunc)(unsigned char key, int x, int y);
typedef void (*IGLIdleFunc)(void);

/* Inicializa GLUT y crea la ventana. Debe llamarse antes de iglRun().
 * argc/argv se pasan directamente a glutInit.
 * logical_w/h: tamaño lógico usado por la proyección ortográfica interna.
 */
void iglInit(int *argc, char **argv, int logical_w, int logical_h, const char *title);

/* Registran callbacks de usuario (pueden ser NULL para usar comportamiento por defecto). */
void iglSetDisplayFunc(IGLDisplayFunc f);
void iglSetReshapeFunc(IGLReshapeFunc f);
void iglSetKeyboardFunc(IGLKeyboardFunc f);
void iglSetIdleFunc(IGLIdleFunc f);

/* Devuelven el tamaño lógico configurado en iglInit */
int iglGetLogicalWidth(void);
int iglGetLogicalHeight(void);


/* Entra al loop principal de GLUT. Bloqueante. */
void iglRun(void);

/* Dibuja un círculo (utilidad exportada). */
void iglDrawCircle(float x, float y, float rad, int n_seg);

/* Testing / utility helpers */
/* Cambia el tamaño lógico sin inicializar GLUT (útil para tests). */
void iglSetLogicalSize(int w, int h);

/* Indican si se registró un callback (1) o no (0). Útiles para tests. */
int iglHasDisplayFunc(void);
int iglHasReshapeFunc(void);
int iglHasKeyboardFunc(void);
int iglHasIdleFunc(void);

#ifdef __cplusplus
}
#endif

#endif /* IGL_INTERFACE_H */
