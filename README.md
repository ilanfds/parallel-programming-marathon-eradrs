# Parallel Programming Marathon — ERAD/RS 2026

Solutions developed for the **Parallel Programming Marathon** held at
**ERAD/RS 2026** (Escola Regional de Alto Desempenho — Rio Grande do Sul).
The contest consists of three problems, each with a sequential reference
implementation; submissions are scored by the **speedup** they achieve over
that reference (`time_sequential / time_yours`), measured by the judging
environment with the `time` utility.

See [`problemset.pdf`](problemset.pdf) for the full problem statements.

## Team

- **Ilan Fernandez**
- **Lucas Pimentel**
- **Vitor Bedin**

## Judging Environment

| Component | Specification |
| --- | --- |
| GPU | NVIDIA Tesla V100-SXM2-32GB |
| GPU driver | 560.35.03 |
| CUDA | 12.6 |
| CPU | 2× Intel Xeon Gold 6252 @ 2.10GHz (48 cores total) |
| RAM | 371 GiB |

## Results

| Problem | Topic | Hardware target | Speedup achieved |
| --- | --- | --- | --- |
| **A** | N-body simulation | GPU (V100) | **425×** |
| **B** | Tree center | CPU (48 cores) | **1569×** |
| **C** | Submatrices summing to *k* | CPU (48 cores) | **94×** |

## The Problems

### Problem A — N-body Simulation

Simulate a system of *N* particles in 3D space under mutual gravitational
attraction. Each particle is described by six `float` values (position
`x, y, z` and velocity `vx, vy, vz`). The simulation runs for a fixed number
of iterations; in each iteration, every body's velocity is updated using the
gravitational force exerted by all other bodies, and positions are integrated
afterwards.

The input and output are binary and byte-compared against the reference
implementation, which makes preserving exact floating-point arithmetic
non-trivial.

### Problem B — Tree Center

Given a tree with *N* nodes (and *N − 1* undirected edges), find the
**center**: the vertex with the smallest eccentricity, where eccentricity is
the maximum distance to any other vertex. The official test cases guarantee
a unique center.

The reference implementation runs a BFS from every vertex on top of an
adjacency matrix, giving **O(N³)** time and **O(N²)** memory.

### Problem C — Number of Submatrices Summing to k

Given an `M × N` integer matrix `A` (with `M, N < 1000` and entries in
`[-1000, 1000]`) and an integer target `k` (with `|k| ≤ 10⁸`), count how many
non-empty submatrices have a sum equal to `k`. This is the 2D generalization
of LeetCode 560 (Subarray Sum Equals K).

## Solutions

### Problem A — `nbody.cu` (CUDA)

The N-body force computation is the textbook example of a workload where
GPUs shine: massively parallel, regular memory access, and high arithmetic
intensity.

Key techniques:

- **One CUDA thread per body** computing the total force exerted on that
  body in parallel.
- **Shared-memory tiling:** each thread block cooperatively loads a tile of
  256 bodies into `__shared__` memory; threads in the block then reuse the
  cached tile, dramatically reducing global-memory traffic.
- **Two kernels per iteration** (`update_velocities`, `update_positions`)
  to avoid races between force computation and position integration.
- **Byte-exact output preservation:** the reference uses `1 / sqrt(...)`
  with the implicit `float → double` promotion of `<math.h>`. The kernel
  mirrors this exactly with `1.0 / sqrt((double) sqrd_dist)`, and is
  compiled with `--fmad=false` so the compiler does not fuse multiply-adds
  in a way that diverges from the CPU reference. This is what makes the
  GPU output match the reference byte-for-byte under `cmp`.

### Problem B — `center.c` (OpenMP)

Rather than parallelizing the O(N³) brute-force reference, we replaced the
algorithm with the classical **Jordan leaf-peeling algorithm**, which solves
the problem in **O(N)**:

1. Identify all current leaves (vertices of degree 1).
2. Remove them simultaneously and decrement the degree of their neighbors.
3. Repeat until 1 or 2 vertices remain — this is the center.

Concrete choices:

- **Compressed Sparse Row (CSR)** adjacency representation, built in a
  single pass over the input edges (count degrees → prefix sum →
  populate neighbor list with a cursor).
- **Round-based peeling** with two parallel phases per round:
  Phase 1 identifies the current leaves into a per-round buffer (using
  `#pragma omp atomic capture` to obtain a unique slot); Phase 2 removes
  them in parallel and atomically decrements neighbor degrees.
- The implicit barrier at the end of each `parallel for` synchronizes
  rounds — no explicit `#pragma omp barrier` is required.

Most of the speedup comes from the algorithmic change (O(N³) → O(N));
the OpenMP parallelism multiplies on top of that.

### Problem C — `main.cpp` (OpenMP, C++)

The standard reduction to 1D is used: for every pair of columns `(j, l)`,
treat the row sums between those columns as a 1D array and count subarrays
whose sum equals `k` using a hashmap of prefix sums. This yields
**O(M² · N)** time.

Key techniques:

- **Custom open-addressing hashmap** (linear probing, MurmurHash3-style
  finalizer) replacing `std::unordered_map`. With at most `M + 1 ≤ 1000`
  distinct keys per column-pair, a fixed capacity of 2048 keeps the load
  factor under 50% and the entire table comfortably in L1/L2 cache. This
  alone is roughly **5× faster** than `std::unordered_map` on this workload
  (no heap allocations, no pointer chasing, better hash distribution).
- **OpenMP `parallel for` over the outer column loop** with
  `reduction(+:ans)` and `schedule(dynamic, 4)`. Dynamic scheduling is
  required because the inner work is triangular: pair `(1, 1)` does much
  more work than pair `(N, N)`.
- The hashmap is declared **inside** the parallel loop, which makes it
  thread-private automatically — no manual `private(...)` clause and no
  shared-state hazards.
- `ios_base::sync_with_stdio(false)` and `cin.tie(NULL)` for fast input
  parsing.

## Repository Layout

```
.
├── A/                 N-body simulation
│   ├── A              execution script
│   ├── Makefile
│   └── nbody.cu       CUDA solution
│
├── B/                 Tree center
│   ├── B              execution script
│   ├── Makefile
│   └── center.c       OpenMP solution
│
├── C/                 Submatrices summing to k
│   ├── C              execution script
│   ├── Makefile
│   └── main.cpp       OpenMP solution
│
├── problemset.pdf     full problem statements
└── README.md
```

Each problem folder is self-contained: the judge runs `make` (using the
`all` rule) and then executes the script with the same name as the
problem (`A`, `B`, or `C`), feeding the test case on standard input and
comparing standard output against the reference.

## Building and Running Locally

Each problem is built and executed independently:

```bash
# Problem A (requires nvcc and a CUDA-capable GPU)
cd A && make
./A < some_input.bin > my_output.bin

# Problem B (requires gcc with OpenMP)
cd B && make
./B < some_input.txt

# Problem C (requires g++ with OpenMP)
cd C && make
./C < some_input.txt
```

To match the judging environment locally on a different GPU, change
`-arch=sm_70` in `A/Makefile` to your card's compute capability
(for example, `sm_86` for an RTX 3060). The judge uses `sm_70` for the V100.
