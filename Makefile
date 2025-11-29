CC = gcc
AR = ar
ARFLAGS = rcs
CFLAGS = -Wall -g -I. -IClases -IAlgorithms -IOpenGLDrawing
LDFLAGS = -lGL -lGLU -lglut -lm

# --- Directorios ---
CLASES_DIR = Clases
ALGORITHMS_DIR = Algorithms
OPENGL_DIR = OpenGLDrawing

# --- Nombres de Archivos ---

# Ejecutables
MAIN_TARGET = virus_simulator
EXAMPLE_TARGET = example_igl
TEST_TARGET = test_interface

# Librería de Interfaz OpenGL
LIB_INTERFACE = libigl.a

# Archivos de la interfaz (OpenGLDrawing/)
INTERFACE_SRC = $(OPENGL_DIR)/interface.c
INTERFACE_OBJ = $(OPENGL_DIR)/interface.o

# Archivos de las clases (Clases/)
CLASES_SRCS = $(CLASES_DIR)/Person.c $(CLASES_DIR)/Regions.c $(CLASES_DIR)/Virus.c
CLASES_OBJS = $(CLASES_DIR)/Person.o $(CLASES_DIR)/Regions.o $(CLASES_DIR)/Virus.o

# Archivos de algoritmos (Algorithms/)
ALGORITHMS_SRCS = $(ALGORITHMS_DIR)/Algorithms.c
ALGORITHMS_OBJS = $(ALGORITHMS_DIR)/Algorithms.o

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
ALL_OBJS = $(INTERFACE_OBJ) $(CLASES_OBJS) $(ALGORITHMS_OBJS) $(MAIN_OBJS) $(EXAMPLE_OBJS) $(TEST_OBJS)

.PHONY: all lib main example test clean run run-main run-example run-test

# --- Targets Principales ---

# Target por defecto: Compila todas las aplicaciones
all: $(MAIN_TARGET) $(EXAMPLE_TARGET) $(TEST_TARGET)

# --- Librería de Interfaz (libigl.a) ---

lib: $(LIB_INTERFACE)

$(LIB_INTERFACE): $(INTERFACE_OBJ) $(CLASES_OBJS) $(ALGORITHMS_OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(INTERFACE_OBJ): $(INTERFACE_SRC) $(OPENGL_DIR)/interface.h
	$(CC) $(CFLAGS) -c $< -o $@

# --- Compilación de Clases ---

$(CLASES_DIR)/Person.o: $(CLASES_DIR)/Person.c $(CLASES_DIR)/Person.h
	$(CC) $(CFLAGS) -c $< -o $@

$(CLASES_DIR)/Regions.o: $(CLASES_DIR)/Regions.c $(CLASES_DIR)/Regions.h
	$(CC) $(CFLAGS) -c $< -o $@

$(CLASES_DIR)/Virus.o: $(CLASES_DIR)/Virus.c $(CLASES_DIR)/Virus.h
	$(CC) $(CFLAGS) -c $< -o $@

# --- Compilación de Algoritmos ---

$(ALGORITHMS_DIR)/Algorithms.o: $(ALGORITHMS_DIR)/Algorithms.c $(ALGORITHMS_DIR)/Algorithms.h
	$(CC) $(CFLAGS) -c $< -o $@

# --- Aplicación Principal (virus_simulator) ---

main: $(MAIN_TARGET)

$(MAIN_TARGET): $(MAIN_OBJS) $(LIB_INTERFACE)
	$(CC) $(CFLAGS) $(MAIN_OBJS) -L. -ligl $(LDFLAGS) -o $@

main_app.o: main_app.c $(OPENGL_DIR)/interface.h
	$(CC) $(CFLAGS) -c $< -o $@

DAO_General.o: DAO_General.c DAO_General.h
	$(CC) $(CFLAGS) -c $< -o $@

# --- Aplicación de Ejemplo (example_igl) ---

example: $(EXAMPLE_TARGET)

$(EXAMPLE_TARGET): $(EXAMPLE_OBJS) $(LIB_INTERFACE)
	$(CC) $(CFLAGS) $(EXAMPLE_OBJS) -L. -ligl $(LDFLAGS) -o $@

example_app.o: example_app.c $(OPENGL_DIR)/interface.h
	$(CC) $(CFLAGS) -c $< -o $@

# --- Aplicación de Prueba (test_interface) ---

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS) $(LIB_INTERFACE)
	$(CC) $(CFLAGS) $(TEST_OBJS) -L. -ligl $(LDFLAGS) -o $@

test_interface.o: test_interface.c $(OPENGL_DIR)/interface.h
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

# --- Target de Limpieza ---

clean:
	rm -f $(ALL_OBJS) $(LIB_INTERFACE) $(MAIN_TARGET) $(EXAMPLE_TARGET) $(TEST_TARGET)
	rm -rf $(CLASES_DIR)/*.o $(ALGORITHMS_DIR)/*.o $(OPENGL_DIR)/*.o# --- Limpieza ---

clean:
	rm -f $(ALL_OBJS) $(LIB_INTERFACE) $(MAIN_TARGET) $(EXAMPLE_TARGET) $(TEST_TARGET)