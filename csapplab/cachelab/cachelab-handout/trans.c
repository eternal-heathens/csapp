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
 void Matrix_32(int M, int N, int A[N][M], int B[M][N]);
 void Matrix_64(int M, int N, int A[N][M], int B[M][N]);
 void Matrix_61(int M, int N, int A[N][M], int B[M][N]);
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    switch(M){
        case 32:
            Matrix_32(M,N,A,B);
            break;
        case 64:
            Matrix_64(M,N,A,B);
            break;
        case 61:
            Matrix_61(M,N,A,B);
    }
}

void Matrix_61(int M, int N, int A[N][M], int B[M][N]){
    int i, j;
    int block_size = 13;
    for (i = 0; i < N; i+=block_size) {
        for (j = 0; j < M; j+=block_size) {
            for(int x = i; x < i+block_size && x < N; x++){
                for(int y = j; y < j+block_size && y < M; y++){
                    B[y][x] = A[x][y];
                }
            }
        }
    }    
}

void Matrix_64(int M, int N, int A[N][M], int B[M][N]){
    int i, j, tmp,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7;;
    // int block_size = 4;
    for (i = 0; i < N; i+=8) {
        for (j = 0; j < M; j+=8) {
            for(int k = i+4; i < k; i++){
                tmp  = A[i][j];
                tmp1 = A[i][j+1];
                tmp2 = A[i][j+2];
                tmp3 = A[i][j+3];
                tmp4 = A[i][j+4];
                tmp5 = A[i][j+5];
                tmp6 = A[i][j+6];
                tmp7 = A[i][j+7];



                B[j][i] = tmp;
                B[j+1][i] = tmp1;
                B[j+2][i] = tmp2;
                B[j+3][i] = tmp3;
                B[j][i+4] = tmp4;
                B[j+1][i+4] = tmp5;
                B[j+2][i+4] = tmp6;
                B[j+3][i+4] = tmp7;
            }
            i-=4;
            
            //为了B的2号能与A的3号能按行先读取暂存值再替换标志，A需要按列读取
            for(int k = j+4; j < k; j++){
                tmp = A[i+4][j];
                tmp1 = A[i+5][j];
                tmp2 = A[i+6][j];
                tmp3 = A[i+7][j];

                //B 按行读取
                tmp4 = B[j][i+4];
                tmp5 = B[j][i+5];
                tmp6 = B[j][i+6];
                tmp7 = B[j][i+7];

                //A3号4x4到B2号 4x4
                B[j][i+4] = tmp;
                B[j][i+5] = tmp1;
                B[j][i+6] = tmp2;
                B[j][i+7] = tmp3;


                B[j+4][i] = tmp4;
                B[j+4][i+1] = tmp5;
                B[j+4][i+2] = tmp6;
                B[j+4][i+3] = tmp7;
            }

        
            j-=4;
            for(int k = i+4; i < k; i++){
                tmp  = A[i+4][j+4];
                tmp1 = A[i+4][j+5];
                tmp2 = A[i+4][j+6];
                tmp3 = A[i+4][j+7];


                B[j+4][i+4] = tmp;
                B[j+5][i+4] = tmp1;
                B[j+6][i+4] = tmp2;
                B[j+7][i+4] = tmp3;
            }

            i-=4;
        
        }
    }    
    
}

void Matrix_32(int M, int N, int A[N][M], int B[M][N]){
    int i, j, tmp,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7;
    for (i = 0; i < N; i+=8) {
        for (j = 0; j < M; j+=8) {
            for(int k = i+8; i < k; i++){
                tmp  = A[i][j];
                tmp1 = A[i][j+1];
                tmp2 = A[i][j+2];
                tmp3 = A[i][j+3];
                tmp4 = A[i][j+4];
                tmp5 = A[i][j+5];
                tmp6 = A[i][j+6];
                tmp7 = A[i][j+7];

                B[j][i] = tmp;
                B[j+1][i] = tmp1;
                B[j+2][i] = tmp2;
                B[j+3][i] = tmp3;
                B[j+4][i] = tmp4;
                B[j+5][i] = tmp5;
                B[j+6][i] = tmp6;
                B[j+7][i] = tmp7;
            }
            i-=8;
        }
    }    
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

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
    // registerTransFunction(trans, trans_desc); 

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

