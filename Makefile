CC = gcc
LDFLAGS = -lglut -lGLU -lGL -lm -lrt -pthread
OBJECTS = main
EXECUTABLE = main

.PHONY: all clean run

all: $(OBJECTS)

main: main.c
	$(CC) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJECTS)

run: $(EXECUTABLE)
	./$(EXECUTABLE) $(ARGS)
