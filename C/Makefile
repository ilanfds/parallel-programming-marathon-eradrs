FLAGS=-O3
EXEC=main
CC=g++

ALL: $(EXEC)

$(EXEC):
	$(CC) $(FLAGS) $(EXEC).cpp -c -o $(EXEC).o
	$(CC) $(FLAGS) $(EXEC).o  -o $(EXEC)

clean:
	rm -rf $(EXEC) *.o
