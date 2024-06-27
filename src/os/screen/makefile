# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# Object files
OBJS_MAIN = main.o screen.o
OBJS_TEST = test.o screen.o
OBJS_UIO = uio.o

# Default target
all: main

# Targets
main: $(OBJS_MAIN)
	$(CC) $(CFLAGS) -o main $(OBJS_MAIN)

test: $(OBJS_TEST)
	$(CC) $(CFLAGS) -o test $(OBJS_TEST)

uio: $(OBJS_UIO)
	$(CC) $(CFLAGS) -o uio $(OBJS_UIO)

# Rules to generate .o files
main.o: main.c
	$(CC) $(CFLAGS) -c main.c

test.o: test.c
	$(CC) $(CFLAGS) -c test.c

screen.o: screen.c
	$(CC) $(CFLAGS) -c screen.c

uio.o: uio.c
	$(CC) $(CFLAGS) -c uio.c
	
# Clean target
clean:
	rm -f *.o main test

# Phony targets
.PHONY: all clean
