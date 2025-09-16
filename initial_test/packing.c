#include "packing.h"

/*
 * Упаковка A: просто копируем блок 4xK в row-major порядке
 */
void pack_A_4xK_rowmajor(int8_t* packed_a, const int8_t* a, size_t lda, size_t k) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < k; ++j) {
            packed_a[i * k + j] = a[i * lda + j];
        }
    }
}

/*
 * Упаковка B: берем блок Kx4 из исходной матрицы и сохраняем
 * его в буфер в column-major порядке.
 */
void pack_B_Kx4_colmajor(int8_t* packed_b, const int8_t* b, size_t ldb, size_t k) {
    for (int j = 0; j < 4; ++j) {       // Идем по 4-м столбцам блока
        for (int i = 0; i < k; ++i) {   // Идем по K строкам
            // Это классическая операция транспонирования
            packed_b[j * k + i] = b[i * ldb + j];
        }
    }
}