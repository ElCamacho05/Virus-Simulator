CC = gcc
AR = ar
ARFLAGS = rcs
CFLAGS = -Wall -g
LDFLAGS = -lGL -lGLU -lglut -lm

LIBNAME = libigl.a
TARGET = example

SRCS = interface.c example_app.c
OBJS = interface.o example_app.o

.PHONY: all lib example clean run

all: example

lib: $(LIBNAME)

$(LIBNAME): interface.o
	$(AR) $(ARFLAGS) $@ $^

interface.o: interface.c interface.h
	$(CC) $(CFLAGS) -c interface.c -o $@

example: example_app.o $(LIBNAME)
	$(CC) $(CFLAGS) example_app.o -L. -ligl $(LDFLAGS) -o $(TARGET)

example_app.o: example_app.c interface.h
	$(CC) $(CFLAGS) -c example_app.c -o $@

clean:
	rm -f *.o $(LIBNAME) $(TARGET)

run: example
	./$(TARGET)

test: lib test_interface
	./test_interface

test_interface: test_interface.o $(LIBNAME)
	$(CC) $(CFLAGS) test_interface.o -L. -ligl $(LDFLAGS) -o test_interface

test_interface.o: test_interface.c interface.h
	$(CC) $(CFLAGS) -c test_interface.c -o test_interface.o
