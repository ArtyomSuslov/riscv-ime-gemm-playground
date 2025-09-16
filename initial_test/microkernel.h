#ifndef MICROKERNEL_H
#define MICROKERNEL_H

#include <stdint.h>
#include <stddef.h>

/*
 * Объявление нашей ассемблерной функции.
 * Она будет умножать матрицу A (4xK) на матрицу B (Kx4)
 * и добавлять результат к матрице C (4x4).
 */
void gemm_microkernel_4x4_int8_int32(
    int32_t* c,       // Указатель на начало блока 4x4 в матрице C
    const int8_t* a,  // Указатель на УПАКОВАННУЮ матрицу A (4xK)
    const int8_t* b,  // Указатель на УПАКОВАННУЮ матрицу B (Kx4)
    size_t k,         // Размер K. Мы будем итерироваться по нему.
    size_t ldc        // Leading dimension of C (шаг до следующей строки в C)
);

#endif // MICROKERNEL_H