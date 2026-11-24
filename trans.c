/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"


int is_transpose(int M, int N, int A[N][M], int B[M][N]);


/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */

#define for_bi8 for (int bi = 0; bi < N; bi += 8)
#define for_bj8 for (int bj = 0; bj < M; bj += 8)
#define for_k04 for (int k = 0; k < 4; k++)
#define for_k08 for (int k = 0; k < 8; k++)

char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    int t[8];
    // 由于 (M,N) 只会取三种尺寸，考虑单独对(64,64)优化
    if (M == 64 && N == 64) {
        for_bi8 {
            for_bj8 {
                //  (P Q
                //   R S)
                for (int i = bi; i < bi + 4; i++) {
                    for_k08 t[k] = A[i][bj+k];
                    // 写入 P^T
                    for_k04 B[bj+k][i] = t[k];
                    // 暂存 Q^T
                    for_k04 B[bj+k][i+4] = t[k+4];
                }
                for (int j = 0; j < 4; j++) {
                    // 读 R
                    for_k04 t[k+4] = A[bi+k+4][bj+j];
                    // 取出暂存
                    for_k04 t[k] = B[bj+j][bi+k+4];
                    //  完成 R^T
                    for_k04 B[bj+j][bi+k+4] = t[k+4];
                    // 将暂存数据移到正确位置
                    for_k04 B[bj+j+4][bi+k] = t[k];
                }
                // 写入 S^T
                for (int i = bi + 4; i < bi + 8; i++) {
                    for_k04 t[k+4] = A[i][bj+k+4];
                    for_k04 B[bj+k+4][i] = t[k+4];
                }
            }
        }
        return;
    }
    // 以下代码在 (32, 32) 和 (61, 67)上已满足满分要求
    for_bi8 {
        for_bj8 {
            for (int i = bi; i < N && i < bi + 8; i++) {
                for (int j = bj; j < M && j < bj + 8; j++) t[j-bj] = A[i][j];
                for (int j = bj; j < M && j < bj + 8; j++) B[j][i] = t[j-bj];
            }
        }
    }
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */
char transpose_block_8_desc[] = "切分成8*8";
void transpose_block_8(int M, int N, int A[N][M], int B[M][N]) {
    for (int bi = 0; bi < N; bi += 8) {
        for (int bj = 0; bj < M; bj += 8) {
            for (int i = bi; i < N && i < bi + 8; i++) {
                for (int j = bj; j < M && j < bj + 8; j++) {
                    B[j][i] = A[i][j];
                }
            }
        }
    }
}

char transpose_block_8_optimized_desc[] = "读写分离版8*8切分";
void transpose_block_8_optimized(int M, int N, int A[N][M], int B[M][N]) {
    int t[8];
    for (int bi = 0; bi < N; bi += 8) {
        for (int bj = 0; bj < M; bj += 8) {
            for (int i = bi; i < N && i < bi + 8; i++) {
                for (int j = bj; j < M && j < bj + 8; j++) t[j-bj] = A[i][j];
                for (int j = bj; j < M && j < bj + 8; j++) B[j][i] = t[j-bj];
            }
        }
    }
}
/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);
    registerTransFunction(transpose_block_8, transpose_block_8_desc);
    registerTransFunction(transpose_block_8_optimized, transpose_block_8_optimized_desc);
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

