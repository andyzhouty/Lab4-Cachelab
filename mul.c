/*
 * mul.c - Matrix multiply C = A * B
 *
 * Each multiply function must have a prototype of the form:
 * void mul(int M, int N, int A[N][M], int B[M][N], int C[N][N]);
 *
 * A multiply function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

int is_mul(int M, int N, int A[N][M], int B[M][N], int C[N][N]);

/*
 * mul_submit - This is the solution multiply function that you will be
 *     graded on for Part C of the assignment. Do not change the
 *     description string "Multiply submission", as the driver searches
 *     for that string to identify the multiply function to be graded.
 *     C starts out holding garbage, so every element of C must be written.
 */
char mul_submit_desc[] = "Multiply submission";
void mul_submit(int M, int N, int A[N][M], int B[M][N], int C[N][N])
{
    int t[3][4];
    int a0, a1, a2, bkj;
    for (int bi = 0; bi < N; bi += 3) {
        for (int bj = 0; bj < N; bj += 4) {
            // 初始化12个累加器
            for (int j = 0; j < 4; j++) {
                t[0][j] = 0;
                t[1][j] = 0;
                t[2][j] = 0;
            }
            for (int k = 0; k < M; k++) {
                // 读取A的三个元素
                a0 = A[bi][k];
                a1 = 0;
                a2 = 0;
                if (bi + 1 < N)
                    a1 = A[bi+1][k];
                if (bi + 2 < N)
                    a2 = A[bi+2][k];
                // 读取B，并同时更新三个累加器
                for (int j = 0; j < 4 && bj+j < N; j++) {
                    bkj = B[k][bj+j];
                    t[0][j] += a0 * bkj;
                    if (bi + 1 < N) t[1][j] += a1 * bkj;
                    if (bi + 2 < N) t[2][j] += a2 * bkj;
                }
            }
            // 写回C
            for (int j = 0; j < 4 && bj+j < N; j++) {
                C[bi][bj+j] = t[0][j];
                if (bi + 1 < N)
                    C[bi+1][bj+j] = t[1][j];
                if (bi + 2 < N)
                    C[bi+2][bj+j] = t[2][j];
            }
        }
    }
}

/*
 * mul - A simple baseline multiply function, not optimized for the cache.
 */
char mul_desc[] = "Simple i-j-k multiply";
// hits:30657, misses:35908, evictions:35876
void mul(int M, int N, int A[N][M], int B[M][N], int C[N][N])
{
    int i, j, k, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            tmp = 0;
            for (k = 0; k < M; k++)
                tmp += A[i][k] * B[k][j];
            C[i][j] = tmp;
        }
    }
}

char ikj_mul_desc[] = "i-k-j multiply";
// hits:86445, misses:13912, evictions:13880
void ikj_mul(int M, int N, int A[N][M], int B[M][N], int C[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            C[i][j] = 0;
        }
    }
    for (int i = 0; i < N; i++) {
        for (int k = 0; k < M; k++) {
            int aik = A[i][k];
            for (int j = 0; j < N; j++) C[i][j] += aik * B[k][j];
        }
    }
}

char ikj_mul_v2_desc[] = "8-column i-k-j multiply";
// hits:33233, misses:4662, evictions:4630
void ikj_mul_v2(int M, int N, int A[N][M], int B[M][N], int C[N][N]) {
    int t[8];
    for (int i = 0; i < N; i++) {
        for (int bj = 0; bj < N; bj += 8) {
            int len = MIN(8, N - bj);
            // 初始化8个累加器
            for (int j = 0; j < len; j++) t[j] = 0;
            for (int k = 0; k < M; k++) {
                int aik = A[i][k];
                for (int j = 0; j < len; j++) t[j] += aik * B[k][bj+j];
            }
            for (int j = 0; j < len; j++) C[i][bj+j] = t[j];
        }
    }
}

char ikj_mul_v3_desc[] = "two-row-in-a-time ikj multiply";
// hits:62033, misses:4534, evictions:4502
void ikj_mul_v3(int M, int N,
                int A[N][M], int B[M][N], int C[N][N]) {
    int t[2][8];
    for (int bi = 0; bi < N; bi += 2) {
        for (int bj = 0; bj < N; bj += 8) {
            for (int j = 0; j < MIN(8, N - bj); j++) {
                t[0][j] = 0;
                if (bi+1 < N) t[1][j] = 0;
            }

            for (int k = 0; k < M; k++) {
                for (int j = 0; j < MIN(8, N - bj); j++) {
                    t[0][j] += A[bi][k] * B[k][bj+j];
                    if (bi+1 < N)
                        t[1][j] += A[bi+1][k] * B[k][bj+j];
                }
            }

            // 3. 写回 C
            for (int j = 0; j < MIN(8, N - bj); j++) {
                C[bi][bj+j] = t[0][j];

                if (bi+1 < N)
                    C[bi+1][bj+j] = t[1][j];
            }
        }
    }
}

char ikj_mul_v4_desc[] = "3*4 ikj multiply";
// hits:16737, misses:3750, evictions:3718
// 符合满分要求，定稿
void ikj_mul_v4(int M, int N,
                int A[N][M], int B[M][N], int C[N][N]) {
    int t[3][4];
    int a0, a1, a2, bkj;
    for (int bi = 0; bi < N; bi += 3) {
        for (int bj = 0; bj < N; bj += 4) {
            // 初始化12个累加器
            for (int j = 0; j < 4; j++) {
                t[0][j] = 0;
                t[1][j] = 0;
                t[2][j] = 0;
            }
            for (int k = 0; k < M; k++) {
                // 读取A的三个元素
                a0 = A[bi][k];
                a1 = (bi + 1 < N) ? A[bi+1][k] : 0;
                a2 = (bi + 2 < N) ? A[bi+2][k] : 0;
                // 读取B，并同时更新三个累加器
                for (int j = 0; j < MIN(4, N-bj); j++) {
                    bkj = B[k][bj+j];
                    t[0][j] += a0 * bkj;
                    if (bi + 1 < N) t[1][j] += a1 * bkj;
                    if (bi + 2 < N) t[2][j] += a2 * bkj;
                }
            }
            // 写回C
            for (int j = 0; j < MIN(4, N-bj); j++) {
                C[bi][bj+j] = t[0][j];
                if (bi + 1 < N)
                    C[bi+1][bj+j] = t[1][j];
                if (bi + 2 < N)
                    C[bi+2][bj+j] = t[2][j];
            }
        }
    }
}

/*
 * registerFunctions - This function registers your multiply functions
 *     with the driver. At runtime, the driver will evaluate each of the
 *     registered functions and summarize their performance. This is a
 *     handy way to experiment with different multiply strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerMulFunction(mul_submit, mul_submit_desc);

    /* Register any additional multiply functions */
    // registerMulFunction(mul, mul_desc);
    // registerMulFunction(ikj_mul, ikj_mul_desc);
    // registerMulFunction(ikj_mul_v2, ikj_mul_v2_desc);
    // registerMulFunction(ikj_mul_v3, ikj_mul_v3_desc);
    // registerMulFunction(ikj_mul_v4, ikj_mul_v4_desc);
}

/*
 * is_mul - This helper function checks if C is the product of A and B.
 *     You can check the correctness of your multiply by calling it
 *     before returning from the multiply function.
 */
int is_mul(int M, int N, int A[N][M], int B[M][N], int C[N][N])
{
    int i, j, k, num;

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            num = 0;
            for (k = 0; k < M; k++)
                num += A[i][k] * B[k][j];
            if (num != C[i][j])
                return 0;
        }
    }
    return 1;
}
