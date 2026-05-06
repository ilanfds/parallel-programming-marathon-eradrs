#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define N_ITER 10 // number of simulation iterations
#define DT 0.01f // time step
#define SOFTENING 1e-9f // to avoid zero divisors

/*
 * Each body holds coordinate positions (i.e., x, y, and z) and
 * velocities (i.e., vx, vy, and vz).
 */
typedef struct { float x, y, z, vx, vy, vz; } Body;

/*
 * Compute the gravitational impact among all pairs of bodies in 
 * the system.
 */
void body_force(Body *p, float dt, int n) {
  for (int i = 0; i < n; ++i) {
    float fx = 0.0f; 
    float fy = 0.0f; 
    float fz = 0.0f;

    for (int j = 0; j < n; j++) {
      float dx = p[j].x - p[i].x;
      float dy = p[j].y - p[i].y;
      float dz = p[j].z - p[i].z;
      float sqrd_dist = dx*dx + dy*dy + dz*dz + SOFTENING;
      float inv_dist = 1 / sqrt(sqrd_dist);
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

/*
 * Read a binary dataset with initilized bodies.
 */
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

/*
 * Write simulation results into a binary dataset.
 */
void write_dataset(const int nbodies, Body *bodies) {
  int b = fwrite(bodies, nbodies * sizeof(Body), 1, stdout);
  if (b != 1) {
    fprintf(stderr,"\nError writing to output\n");
    exit(EXIT_FAILURE);
  }
}

int main(int argc,char **argv) {
  int nbodies;

  Body *bodies = read_dataset(&nbodies);

  /*
   * At each simulation iteration, interbody forces are computed,
   * and bodies' positions are integrated.
   */
  for (int iter = 0; iter < N_ITER; iter++) {
    body_force(bodies, DT, nbodies);

    for (int i = 0; i < nbodies; i++) {
      bodies[i].x += bodies[i].vx * DT;
      bodies[i].y += bodies[i].vy * DT;
      bodies[i].z += bodies[i].vz * DT;
    }
  }

  write_dataset(nbodies, bodies);

  free(bodies);
  
  exit(EXIT_SUCCESS);
}
