NVCC      = nvcc
NVCCFLAGS = -O3 -arch=sm_70 -Xcompiler -Wall

TARGET = matmul
SRC    = matmul.cu

all: $(TARGET)

$(TARGET): $(SRC)
	$(NVCC) $(NVCCFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) 2048

clean:
	rm -f $(TARGET)

.PHONY: all run clean
