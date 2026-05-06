#include <stdio.h>
#include <stdlib.h>

#define FILE_POINTER stdin

typedef struct { 
	int n;
	int **adj_matrix;
	int *visited;
} Tree_t;



Tree_t *read_edges(FILE *fp) { 
	int a, b, n, i, j;
	int **adj_matrix, *visited;
	Tree_t *tree;

	tree = (Tree_t *)malloc(sizeof(Tree_t));

	fscanf(fp, "%d", &n);
	adj_matrix = (int **)malloc(sizeof(int *)*n);
	visited = (int *)malloc(sizeof(int)*n);
	for (i = 0; i < n; i++) {
		visited[i] = 0;
		adj_matrix[i] = (int *)malloc(sizeof(int)*n);

	}
	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++)
			adj_matrix[i][j] = 0;
	
	fscanf(fp, "%d %d", &a, &b);
	while (!feof(fp)) {
		adj_matrix[a-1][b-1] = 1; //inputs start with 1
		adj_matrix[b-1][a-1] = 1; //replicate adjancency the other direction


		fscanf(fp, "%d %d", &a, &b);


	}

	tree->n = n;
	tree->adj_matrix = adj_matrix;
	tree->visited = visited;
	return(tree);

}

void print_visited(Tree_t *tree) {
	int i;
	for (i = 0; i < tree->n; i++)
		printf("%d: %d ", i+1, tree->visited[i]);
	printf("\n");

} 
int max_distance(Tree_t *tree, int *queue) {
	int n, last, first, max_level, i, x;

	last = 1;
	first = 0;
	n = tree->n;
	max_level = 0;
	for (i = 0; i < n; i++)
		tree->visited[i] = 0;
	tree->visited[queue[0]] = 1;
	while (first < n) {
		x = queue[first++];
		for (i = 0; i < n; i++) {
			if (tree->adj_matrix[x][i] && !tree->visited[i]) {
				queue[last++] = i;
				tree->visited[i] = tree->visited[x] + 1;

			}
		}

	}
	
	for (i = 0; i < n; i++) {
		if (tree->visited[i] > max_level)
			max_level = tree->visited[i];	
	}
	return(max_level - 1);
} 
void print_tree(Tree_t *tree) { //can be used for debugging
	int n = tree->n, **adjs = tree->adj_matrix, i, j;

	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++)
			if (adjs[i][j])
				printf("(%d, %d) ", i+1, j+1);


}
int find_center(Tree_t *tree, int *queue) {
	int n = tree->n, center = -1, i, max_dist, tmp;
	max_dist = n;


	for (i = 0; i < n; i++) {
		queue[0] = i;
		tmp = max_distance(tree, queue);
		if (tmp < max_dist) {
			max_dist = tmp;
			center = i;
		}
	}	

	return(center);

}

int main(void) {
	Tree_t *tree;
	int center, *queue;

	tree = read_edges(FILE_POINTER);
	queue = (int *)malloc(sizeof(int)*tree->n);


	center = find_center(tree, queue);
	printf("%d\n", center+1);

}
