/* test_interface.c
 * Pruebas simples para la API de interface: verifica getters/setters lógicos
 * y registro de callbacks sin usar un framework de tests.
 */

#include <stdio.h>
#include <stdlib.h>
#include "interface.h"

static int tests_run = 0;
static int tests_failed = 0;

#define ASSERT(cond, msg) do { \
    tests_run++; \
    if (!(cond)) { \
        fprintf(stderr, "TEST FAILED: %s (line %d)\n", msg, __LINE__); \
        tests_failed++; \
    } else { \
        fprintf(stdout, "ok: %s\n", msg); \
    } \
} while(0)

/* Dummy callbacks */
void dummy_display(void) {}
void dummy_reshape(int w, int h) {}
void dummy_keyboard(unsigned char k, int x, int y) {}
void dummy_idle(void) {}

int main(void) {
    /* Default logical size should be 500x500 as initialized in interface.c */
    ASSERT(iglGetLogicalWidth() == 500, "default logical width is 500");
    ASSERT(iglGetLogicalHeight() == 500, "default logical height is 500");

    /* Change logical size via helper and check */
    iglSetLogicalSize(640, 480);
    ASSERT(iglGetLogicalWidth() == 640, "logical width set to 640");
    ASSERT(iglGetLogicalHeight() == 480, "logical height set to 480");

    /* Callback registration tests */
    iglSetDisplayFunc(NULL);
    ASSERT(iglHasDisplayFunc() == 0, "no display func registered initially");
    iglSetDisplayFunc(dummy_display);
    ASSERT(iglHasDisplayFunc() == 1, "display func registered");
    iglSetDisplayFunc(NULL);

    iglSetReshapeFunc(NULL);
    ASSERT(iglHasReshapeFunc() == 0, "no reshape func registered initially");
    iglSetReshapeFunc(dummy_reshape);
    ASSERT(iglHasReshapeFunc() == 1, "reshape func registered");
    iglSetReshapeFunc(NULL);

    iglSetKeyboardFunc(NULL);
    ASSERT(iglHasKeyboardFunc() == 0, "no keyboard func registered initially");
    iglSetKeyboardFunc(dummy_keyboard);
    ASSERT(iglHasKeyboardFunc() == 1, "keyboard func registered");
    iglSetKeyboardFunc(NULL);

    iglSetIdleFunc(NULL);
    ASSERT(iglHasIdleFunc() == 0, "no idle func registered initially");
    iglSetIdleFunc(dummy_idle);
    ASSERT(iglHasIdleFunc() == 1, "idle func registered");
    iglSetIdleFunc(NULL);

    fprintf(stdout, "\nTests run: %d, failed: %d\n", tests_run, tests_failed);
    return tests_failed == 0 ? 0 : 1;
}
