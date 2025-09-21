#include "packing.h"

void pack_A_4xK(int8_t* packed_a, const int8_t* a,
                size_t n, size_t k, size_t block_size)
{
    for (size_t i = 0; i < n; i += 4) {                      // по блокам строк
        for (size_t j = 0; j < k; j += block_size) {         // по блокам столбцов
            for (size_t ii = 0; ii < 4; ii++) {              // строки внутри блока
                size_t row = i + ii;
                for (size_t jj = 0; jj < block_size; jj++) { // столбцы внутри блока
                    size_t col = j + jj;
                    if (row < n && col < k) {
                        *packed_a++ = a[row * k + col];      // row-major внутри блока
                    } else {
                        *packed_a++ = 0;                     // паддинг
                    }
                }
            }
        }
    }
}


void pack_B_Kx4(int8_t* packed_b, const int8_t* b,
                size_t k, size_t m, size_t block_size)
{
    for (size_t j = 0; j < m; j += 4) {                      // по блокам столбцов
        for (size_t i = 0; i < k; i += block_size) {         // по блокам строк
            for (size_t jj = 0; jj < 4; jj++) {              // колонки внутри блока
                size_t col = j + jj;
                for (size_t ii = 0; ii < block_size; ii++) { // строки внутри блока
                    size_t row = i + ii;
                    if (row < k && col < m) {
                        *packed_b++ = b[row * m + col];      // column-major внутри блока
                    } else {
                        *packed_b++ = 0;                     // паддинг
                    }
                }
            }
        }
    }
}