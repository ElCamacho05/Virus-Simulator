CC = gcc
AR = ar
ARFLAGS = rcs
CFLAGS = -Wall -g
LDFLAGS = -lGL -lGLU -lglut -lm

# --- Nombres de Archivos ---

# Ejecutables
MAIN_TARGET = virus_simulator
EXAMPLE_TARGET = example_igl
TEST_TARGET = test_interface

# Librería
LIB_INTERFACE = libigl.a

# Archivos de la interfaz (libigl.a)
INTERFACE_SRC = interface.c
INTERFACE_OBJ = interface.o

# Archivos de la aplicación principal
MAIN_SRCS = main_app.c DAO_General.c
MAIN_OBJS = main_app.o DAO_General.o

# Archivos de la aplicación de ejemplo
EXAMPLE_SRCS = example_app.c
EXAMPLE_OBJS = example_app.o

# Archivos de prueba
TEST_SRCS = test_interface.c
TEST_OBJS = test_interface.o

# Todos los objetos generados
ALL_OBJS = $(INTERFACE_OBJ) $(MAIN_OBJS) $(EXAMPLE_OBJS) $(TEST_OBJS)

.PHONY: all lib main example test clean run run-main run-example run-test

# --- Targets Principales ---

# Target por defecto: Compila todas las aplicaciones
all: $(MAIN_TARGET) $(EXAMPLE_TARGET) $(TEST_TARGET)

# --- Librería de Interfaz (libigl.a) ---

lib: $(LIB_INTERFACE)

$(LIB_INTERFACE): $(INTERFACE_OBJ)
	$(AR) $(ARFLAGS) $@ $^

$(INTERFACE_OBJ): $(INTERFACE_SRC) interface.h
	$(CC) $(CFLAGS) -c $< -o $@

# --- Aplicación Principal (virus_simulator) ---

main: $(MAIN_TARGET)

$(MAIN_TARGET): $(MAIN_OBJS) $(LIB_INTERFACE)
	$(CC) $(CFLAGS) $(MAIN_OBJS) -L. -ligl $(LDFLAGS) -o $@

main_app.o: main_app.c interface.h
	$(CC) $(CFLAGS) -c $< -o $@

DAO_General.o: DAO_General.c
	$(CC) $(CFLAGS) -c $< -o $@

# --- Aplicación de Ejemplo (example_igl) ---

example: $(EXAMPLE_TARGET)

$(EXAMPLE_TARGET): $(EXAMPLE_OBJS) $(LIB_INTERFACE)
	$(CC) $(CFLAGS) $(EXAMPLE_OBJS) -L. -ligl $(LDFLAGS) -o $@

example_app.o: example_app.c interface.h
	$(CC) $(CFLAGS) -c $< -o $@

# --- Aplicación de Prueba (test_interface) ---

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS) $(LIB_INTERFACE)
	$(CC) $(CFLAGS) $(TEST_OBJS) -L. -ligl $(LDFLAGS) -o $@

test_interface.o: test_interface.c interface.h
	$(CC) $(CFLAGS) -c $< -o $@

# --- Targets de Ejecución ---

# Ejecuta el programa principal por defecto
run: run-main

run-main: $(MAIN_TARGET)
	./$(MAIN_TARGET)

run-example: $(EXAMPLE_TARGET)
	./$(EXAMPLE_TARGET)

run-test: $(TEST_TARGET)
	./$(TEST_TARGET)

# --- Limpieza ---

clean:
	rm -f $(ALL_OBJS) $(LIB_INTERFACE) $(MAIN_TARGET) $(EXAMPLE_TARGET) $(TEST_TARGET)