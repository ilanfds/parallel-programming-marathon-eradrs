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
    int* src = (int*)calloc(t->num_vertices-1,sizeof(int));
    int* dst = (int*)calloc(t->num_vertices-1,sizeof(int));
    {
        int idx = 0;
        for(;;){
            if(fscanf(stdin, "%d %d", &a, &b)!=2) break;
            a--; b--; // precisamos de 0-indexed
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

int find_center(Tree *t){
    // A ideia aqui eh tirar os vertices folhas (vertices de grau 1)
    // ate que sobre 1 ou 2 vertices (enunciado garante que sempre sobrara
    // apenas 1).
    int* curr_leaves = (int*) malloc((t->num_vertices+1)*sizeof(int));
    int num_leaves;

    int remainder = t->num_vertices;

    while(remainder > 2){
        num_leaves = 0;

        // coloca todas as folhas no buffer
        #pragma omp parallel for
        for(int v = 0 ; v < t->num_vertices ; ++v){
            if(!t->unactive[v] && t->degrees[v] == 1){
                int idx;
                #pragma omp atomic capture
                idx = num_leaves++;
                curr_leaves[idx] = v;
            }
        }

        // remover as folhas e att os graus
        #pragma omp parallel for
        for(int i = 0 ; i < num_leaves ;++i){
            int v =  curr_leaves[i];
            t->unactive[v] = true;

            int start = t->offset[v];
            int end   = t->offset[v+1];
            // itera nos vizinhos
            for(int j = start ; j < end ; ++j){
                int u = t->neighbors[j];
                
                if(t->unactive[u]) continue;
                
                #pragma omp atomic
                t->degrees[u]--;
            }
        }

        remainder -= num_leaves;
    }

    free(curr_leaves);

    int ans=-1;
    #pragma omp parallel for
    for(int v = 0 ; v < t->num_vertices ;++v){
        if(!t->unactive[v]) ans = v;
    }

    return ans+1;
}

int main(int argc, const char * argv[]){
    Tree tree;
    int num_vertices;
    fscanf(stdin,"%d",&num_vertices);
    init_tree(&tree,num_vertices);
    read_input(&tree);

    int x = find_center(&tree);
    fprintf(stdout,"%d\n",x);
    return 0;
}
