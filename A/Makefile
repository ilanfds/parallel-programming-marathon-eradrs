CC=nvcc

# Flags: -arch=sm_70 alvo a Tesla V100 do julgamento
FLAGS=-O3 -arch=sm_70 --fmad=false

# Files to compile
SRC=a.cu

# Executable name
EXE=a


all: $(SRC)
	$(CC) $(FLAGS) $(SRC) -o $(EXE)

clean:
	rm -f $(EXE)
