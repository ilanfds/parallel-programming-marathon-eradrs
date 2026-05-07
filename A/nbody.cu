#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cuda_runtime.h>

#define N_ITER 10 // number of simulation iterations
#define DT 0.01f // time step
#define SOFTENING 1e-9f // to avoid zero divisors


typedef struct { float x, y, z, vx, vy, vz; } Body;

__global__ void update_positions(Body *p, float dt, int n){
    int jump = blockDim.x * gridDim.x;
    int index = threadIdx.x + blockDim.x * blockIdx.x;

    for (int i = index; i < n; i += jump) {
      p[i].x += dt * p[i].vx;
      p[i].y += dt * p[i].vy;
      p[i].z += dt * p[i].vz;
    }

}

__global__ void update_velocities(Body *p, float dt, int n){
    // cada thread sera responsavel por atualizar um numero fixo de particulas

    // cada thread atualiza particulas pulando de jump em jump na lista de particulas
    int jump = blockDim.x * gridDim.x;
    int index = threadIdx.x + blockDim.x * blockIdx.x;

    for (int i = index; i < n; i += jump) {
      float fx = 0.0f, fy = 0.0f, fz = 0.0f;
      for (int j = 0; j < n; j++) {

        float dx, dy, dz;
        dx = p[j].x - p[i].x;
        dy = p[j].y - p[i].y;
        dz = p[j].z - p[i].z;

        float sqrd_dist = dx*dx + dy*dy + dz*dz + SOFTENING;
        float inv_dist = 1.0 / sqrt((double)sqrd_dist);
        float inv_dist3 = inv_dist * inv_dist * inv_dist;

        fx += dx * inv_dist3;
        fy += dy * inv_dist3;
        fz += dz * inv_dist3;
      }

      p[i].vx += dt*fx;
      p[i].vy += dt*fy;
      p[i].vz += dt*fz;

    }
}

Body* read_dataset(int *nbodies) {
  int b = fread(nbodies, sizeof(*nbodies), 1, stdin);
  if (b != 1) {
    fprintf(stderr,"\nError reading nbodie value\n");
    exit(EXIT_FAILURE);
  }
  Body *bodies = (Body *)malloc(*nbodies * sizeof(Body));
  b = fread(bodies, *nbodies * sizeof(Body), 1, stdin);
  if (b != 1) {
    fprintf(stderr,"\nError reading input values\n");
    exit(EXIT_FAILURE);
  }

  return bodies;
}

void write_dataset(const int nbodies, Body *bodies) {
  int b = fwrite(bodies, nbodies * sizeof(Body), 1, stdout);
  if (b != 1) {
    fprintf(stderr,"\nError writing to output\n");
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char **argv) {
  int nbodies;
  Body *h_bodies = read_dataset(&nbodies);

  size_t bytes = nbodies * sizeof(Body);

  Body *d_bodies;
  cudaMalloc(&d_bodies, bytes);
  cudaMemcpy(d_bodies, h_bodies, bytes, cudaMemcpyHostToDevice);

  int block_size = 256;
  int grid_size = (nbodies + block_size - 1) / block_size;

  for (int iter = 0; iter < N_ITER; iter++) {
    update_velocities<<<grid_size, block_size>>>(d_bodies, DT, nbodies);
    update_positions<<<grid_size, block_size>>>(d_bodies, DT, nbodies);
  }

  cudaMemcpy(h_bodies, d_bodies, bytes, cudaMemcpyDeviceToHost);
  cudaFree(d_bodies);

  write_dataset(nbodies, h_bodies);
  free(h_bodies);

  return 0;
}
