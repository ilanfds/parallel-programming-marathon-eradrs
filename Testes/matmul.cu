/*
 * Multiplicacao de matrizes C = A * B em CUDA.
 *
 * Hardware alvo:
 *   GPU: Tesla V100-SXM2-32GB (compute capability 7.0, 80 SMs, 5120 CUDA cores)
 *   CPU: 2x Intel Xeon Gold 6252 @ 2.10GHz (24 cores cada, 48 total)
 *
 * Estrategia:
 *   - Tiling em shared memory com blocos 32x32 (1024 threads = maximo por bloco).
 *   - Cada bloco carrega 1 tile de A e 1 tile de B em shared memory por iteracao
 *     do loop sobre tiles, reduzindo acessos a memoria global em ~32x.
 *   - Cada thread calcula 1 elemento de C.
 *
 * Compilar:
 *   nvcc -O3 -arch=sm_70 matmul.cu -o matmul
 *
 * Rodar:
 *   ./matmul              # default: 2048x2048
 *   ./matmul 4096         # matrizes 4096x4096
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <cuda_runtime.h>

#define TILE 32

#define CUDA_CHECK(call) do {                                       \
    cudaError_t err = (call);                                       \
    if (err != cudaSuccess) {                                       \
        fprintf(stderr, "CUDA erro %s:%d: %s\n",                    \
                __FILE__, __LINE__, cudaGetErrorString(err));       \
        exit(EXIT_FAILURE);                                         \
    }                                                               \
} while (0)

/*
 * Kernel: cada thread (tx, ty) dentro de um bloco (bx, by) calcula
 * C[row][col] onde row = by*TILE + ty e col = bx*TILE + tx.
 *
 * O loop externo varre os tiles ao longo da dimensao K. Em cada iteracao:
 *   1. Carrega A[row][k_tile..k_tile+TILE] e B[k_tile..k_tile+TILE][col]
 *      cooperativamente em shared memory.
 *   2. __syncthreads() pra garantir que todos os dados estejam la.
 *   3. Acumula o produto parcial usando os tiles em shared memory.
 *   4. __syncthreads() antes de carregar o proximo tile.
 */
__global__ void matmul_tiled(const float *A, const float *B, float *C, int N) {
    __shared__ float As[TILE][TILE];
    __shared__ float Bs[TILE][TILE];

    int tx = threadIdx.x;
    int ty = threadIdx.y;
    int row = blockIdx.y * TILE + ty;
    int col = blockIdx.x * TILE + tx;

    float acc = 0.0f;

    int n_tiles = (N + TILE - 1) / TILE;
    for (int t = 0; t < n_tiles; t++) {
        int a_col = t * TILE + tx;
        int b_row = t * TILE + ty;

        As[ty][tx] = (row < N && a_col < N) ? A[row * N + a_col] : 0.0f;
        Bs[ty][tx] = (b_row < N && col < N) ? B[b_row * N + col] : 0.0f;

        __syncthreads();

        #pragma unroll
        for (int k = 0; k < TILE; k++) {
            acc += As[ty][k] * Bs[k][tx];
        }

        __syncthreads();
    }

    if (row < N && col < N) {
        C[row * N + col] = acc;
    }
}

/*
 * Baseline em CPU (single-thread, ordem ijk classica) pra comparar
 * resultado e tempo. Para N grande, e bem lento — esperado.
 */
void matmul_cpu(const float *A, const float *B, float *C, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            float s = 0.0f;
            for (int k = 0; k < N; k++) {
                s += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = s;
        }
    }
}

static double now_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(int argc, char **argv) {
    int N = (argc > 1) ? atoi(argv[1]) : 2048;
    size_t bytes = (size_t)N * N * sizeof(float);

    printf("Multiplicacao de matrizes %dx%d (%.2f MB por matriz)\n",
           N, N, bytes / (1024.0 * 1024.0));

    /* Aloca host */
    float *hA = (float *)malloc(bytes);
    float *hB = (float *)malloc(bytes);
    float *hC_gpu = (float *)malloc(bytes);
    float *hC_cpu = (float *)malloc(bytes);
    if (!hA || !hB || !hC_gpu || !hC_cpu) {
        fprintf(stderr, "malloc falhou\n");
        return EXIT_FAILURE;
    }

    /* Inicializa com valores aleatorios em [0,1) */
    srand(42);
    for (int i = 0; i < N * N; i++) {
        hA[i] = (float)rand() / RAND_MAX;
        hB[i] = (float)rand() / RAND_MAX;
    }

    /* Aloca device */
    float *dA, *dB, *dC;
    CUDA_CHECK(cudaMalloc(&dA, bytes));
    CUDA_CHECK(cudaMalloc(&dB, bytes));
    CUDA_CHECK(cudaMalloc(&dC, bytes));

    /* Copia host -> device (cronometrado separadamente do kernel) */
    double t0 = now_seconds();
    CUDA_CHECK(cudaMemcpy(dA, hA, bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(dB, hB, bytes, cudaMemcpyHostToDevice));
    double t_h2d = now_seconds() - t0;

    /* Configura grid e lanca kernel */
    dim3 block(TILE, TILE);
    dim3 grid((N + TILE - 1) / TILE, (N + TILE - 1) / TILE);

    /* Usa cudaEvent pra medir o kernel com precisao */
    cudaEvent_t e_start, e_stop;
    CUDA_CHECK(cudaEventCreate(&e_start));
    CUDA_CHECK(cudaEventCreate(&e_stop));

    CUDA_CHECK(cudaEventRecord(e_start));
    matmul_tiled<<<grid, block>>>(dA, dB, dC, N);
    CUDA_CHECK(cudaEventRecord(e_stop));
    CUDA_CHECK(cudaEventSynchronize(e_stop));

    float ms_kernel = 0.0f;
    CUDA_CHECK(cudaEventElapsedTime(&ms_kernel, e_start, e_stop));

    /* Copia device -> host */
    t0 = now_seconds();
    CUDA_CHECK(cudaMemcpy(hC_gpu, dC, bytes, cudaMemcpyDeviceToHost));
    double t_d2h = now_seconds() - t0;

    /* GFLOPS: 2*N^3 operacoes (N^3 mul + N^3 add) */
    double gflops = (2.0 * N * N * N) / (ms_kernel / 1000.0) / 1e9;

    printf("\n=== GPU (V100) ===\n");
    printf("  H->D:   %.3f s\n", t_h2d);
    printf("  Kernel: %.3f ms (%.1f GFLOPS)\n", ms_kernel, gflops);
    printf("  D->H:   %.3f s\n", t_d2h);

    /* Baseline CPU — so roda se N for pequeno o bastante (senao demora demais) */
    if (N <= 1024) {
        printf("\n=== CPU (single-thread) ===\n");
        t0 = now_seconds();
        matmul_cpu(hA, hB, hC_cpu, N);
        double t_cpu = now_seconds() - t0;
        double gflops_cpu = (2.0 * N * N * N) / t_cpu / 1e9;
        printf("  Tempo:  %.3f s (%.2f GFLOPS)\n", t_cpu, gflops_cpu);
        printf("  Speedup GPU vs CPU: %.1fx\n", t_cpu * 1000.0 / ms_kernel);

        /* Verifica corretude */
        double max_diff = 0.0;
        for (int i = 0; i < N * N; i++) {
            double d = fabs((double)hC_gpu[i] - (double)hC_cpu[i]);
            if (d > max_diff) max_diff = d;
        }
        printf("  Diferenca maxima GPU vs CPU: %.2e\n", max_diff);
    } else {
        printf("\n(CPU baseline pulado: N=%d e grande demais para single-thread)\n", N);
    }

    /* Cleanup */
    cudaEventDestroy(e_start);
    cudaEventDestroy(e_stop);
    cudaFree(dA); cudaFree(dB); cudaFree(dC);
    free(hA); free(hB); free(hC_gpu); free(hC_cpu);

    return 0;
}
