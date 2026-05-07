#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>

/*
estruturas de dados:

vetor de adjacencias, eh um vetor de vetores, onde cada elemento e a adjacencia
daquele vertice
_ 
1 -> null
2 -> [3,9]
3 -> [1,2,4]
4 -> [3]
...
_

1 2 3 4 5

*/

// vamos usar uma CSR
typedef struct{
    int num_vertices;
    int*  offset;
    int*  neighbors;
    int*  degrees;
    bool* unactive;
} Tree;

void printTree(Tree t){
    printf("Tree:{\n");
    printf("\tnum_vertices = %d\n",t.num_vertices);
    int max = (10 < t.num_vertices-1) ? 10 : t.num_vertices-1;
    printf("\toffset = [");
    for(int i = 0 ; i<max ; ++i ) printf("%d,",t.offset[i]);
    printf("]\n");
    printf("\tneighbors = [");
    for(int i = 0 ; i<max ; ++i ) printf("%d,",t.neighbors[i]);
    printf("]\n");
    printf("\tdegrees = [");
    for(int i = 0 ; i<max ; ++i ) printf("%d,",t.degrees[i]);
    printf("]\n");
    printf("}\n");
}

void init_tree(Tree *t,int num_vertices){
    int num_arestas = num_vertices-1;
    t->num_vertices = num_vertices;
    t->offset    = (int* )calloc(num_vertices+1,sizeof(int) );
    t->neighbors = (int* )calloc(2*num_arestas,sizeof(int) ); // bidirecional
    t->degrees   = (int* )calloc(num_vertices,sizeof(int) );
    t->unactive  = (bool*)malloc(num_vertices*sizeof(bool));
    #pragma omp parallel for
    for(int i = 0; i < num_vertices; ++i)
        t->unactive[i] = false;
}

void read_input(Tree *t){
    int a,b;
    int* src = (int*)calloc(t->num_vertices,sizeof(int));
    int* dst = (int*)calloc(t->num_vertices,sizeof(int));
    {
        int idx = 0;
        for(;;){
            if(fscanf(stdin, "%d %d", &a, &b)!=2) break;
            a--; b--; // need 0 indexed vertices
            src[idx] = a;
            dst[idx] = b;
            idx++;
            t->degrees[a] ++;
            t->degrees[b] ++;
        }
    }
    t->offset[0] = 0;
    for(int v = 1 ; v < t->num_vertices + 1 ; ++v){
        t->offset[v] = t->offset[v-1] + t->degrees[v-1];
    }

    int* aux = (int*)calloc(t->num_vertices,sizeof(int));
    
    for(int idx = 0 ; idx < t->num_vertices - 1 ;++idx){
        int a = src[idx];
        int b = dst[idx];
        
        t->neighbors[t->offset[a]+aux[a]] = b;
        aux[a]++;
        
        t->neighbors[t->offset[b]+aux[b]] = a;
        aux[b]++;
    }
}

int main(int argc, const char * argv[]){
    Tree tree;
    int num_vertices;
    fscanf(stdin,"%d",&num_vertices);
    init_tree(&tree,num_vertices);
    read_input(&tree);

    printTree(tree);


    return 0;
}
