import sys
import random

def generate_matrix(filename, key, height, width, max_val):
    with open(filename, 'w') as f:
        # Escreve o cabeçalho no formato esperado pelo fscanf: key height width
        f.write(f"{key} {height} {width}\n")
        
        # Escreve a matriz linha por linha para otimizar o uso de memória
        for _ in range(height):
            # Gera uma linha de valores aleatórios entre 0 e max_val
            row = [str(random.randint(0, max_val)) for _ in range(width)]
            f.write(" ".join(row) + "\n")

if __name__ == "__main__":
    if len(sys.argv) != 6:
        print("Uso: python gera_matriz.py <arquivo_saida> <key> <height> <width> <max_val>")
        sys.exit(1)
    
    filename = sys.argv[1]
    key = int(sys.argv[2])
    height = int(sys.argv[3])
    width = int(sys.argv[4])
    max_val = int(sys.argv[5])
    
    print(f"Gerando matriz {width}x{height} no arquivo '{filename}'...")
    generate_matrix(filename, key, height, width, max_val)
    print("Concluído com sucesso!")
