CC=nvcc

# Flags: -arch=sm_70 alvo a Tesla V100 do julgamento
FLAGS=-O3 -arch=sm_70 --fmad=false

# Files to compile
SRC=nbody.cu

# Executable name
EXE=nbody


all: $(SRC)
	$(CC) $(FLAGS) $(SRC) -o $(EXE)

clean:
	rm -f $(EXE)
