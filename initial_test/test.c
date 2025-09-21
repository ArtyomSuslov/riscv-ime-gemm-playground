#include <stdio.h>
#include <stdlib.h>
#include "microkernel.h"
#include "packing.h"

// Наивная реализация для проверки
void naive_gemm_4x4(int32_t* c, const int8_t* a, const int8_t* b, size_t k) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            c[i * 4 + j] = 0;
            for (int p = 0; p < k; ++p) {
                c[i * 4 + j] += (int32_t)a[i * k + p] * (int32_t)b[p * 4 + j];
            }
        }
    }
}

int main() {
    const size_t K = 8; // K должно быть кратно 8
    
    int8_t A[4][K];
    int8_t B[K][4];
    int32_t C_asm[4][4] = {0};
    int32_t C_naive[4][4] = {0};

    for(int i=0; i<4*K; ++i) ((int8_t*)A)[i] = i % 10 + 1;
    for(int i=0; i<K*4; ++i) ((int8_t*)B)[i] = i % 7 + 1;

    // Упаковка
    int8_t* packed_A = (int8_t*)malloc(4 * K * sizeof(int8_t));
    int8_t* packed_B = (int8_t*)malloc(K * 4 * sizeof(int8_t));

    pack_A_4xK_rowmajor(packed_A, (const int8_t*)A, K, K);
    pack_B_Kx4_colmajor(packed_B, (const int8_t*)B, 4, K);

    // Вызов ассемблерного ядра
    gepp_microkernel_4x4_int8_int32((int32_t*)C_asm, packed_A, packed_B, K, 4);

    // Вызов наивной реализации для проверки
    naive_gemm_4x4((int32_t*)C_naive, (const int8_t*)A, (const int8_t*)B, K);

    // Сравнение результатов
    printf("Result from ASM kernel:\n");
    int all_good = 1;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            int a = C_asm[i][j];
            int b = C_naive[i][j];
            if (a != b) {
                all_good = 0;
                printf("(%d,%d): %6d != %6d\n", i, j, a, b);
            }
        }
    }
    if (all_good) {
        printf("Correct gemm_microkernel_4x4_int8_int32\n");
    } else {
        printf("Some differencies\n");
    }

    free(packed_A);
    free(packed_B);

    return 0;
}