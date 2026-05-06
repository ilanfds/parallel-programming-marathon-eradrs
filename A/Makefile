COMPILER=gcc

# Flags
CFLAGS=-O3 -Wall -std=gnu11 -lm

# Files to compile
OBJECTS=nbody.c

# Executable name
EXE=nbody


all: $(OBJECTS)
	$(COMPILER) $(OBJECTS) $(CFLAGS) -o $(EXE)

run:
	./$(EXE)

clean:
	rm $(EXE)

