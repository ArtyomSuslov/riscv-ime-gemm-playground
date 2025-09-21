#include <stdio.h>
#include <string.h>
#include "kernel_v.h"
#include "microkernel_v.h"
#include "../packing/packing.h"
#include "../extra_funcs/print_matrix.h"

#define MAX_VLEN 256

void gemm_int8_int32_v(
    int32_t *C,
    int8_t *A,
    int8_t *B,
    size_t m,
    size_t n,
    size_t k)
{
    
    size_t max_m_n_for_vlen = cbrt(MAX_VLEN);
    
    // TODO: добавить логику автоматического определения нужных размеров блока
    size_t mb = 4, nb = 4, kb = 8;

    // Упаковка матриц в соответсвиями с этоми размерами
    size_t m_pad = ((m + mb - 1) / mb) * mb;
    size_t n_pad = ((n + nb - 1) / nb) * nb;
    size_t k_pad = ((k + kb - 1) / kb) * kb;

    size_t size_packed_A = m_pad * k_pad;
    size_t size_packed_B = k_pad * n_pad;
    size_t size_packed_C = m_pad * n_pad;

    int8_t *packed_A = (int8_t*)malloc(size_packed_A * sizeof(int8_t));
    int8_t *packed_B = (int8_t*)malloc(size_packed_B * sizeof(int8_t));
    int32_t *C_padded = (int32_t*)malloc(size_packed_C * sizeof(int32_t));

    pack_A_4xK(packed_A, A, m, k, kb);
    pack_B_Kx4(packed_B, B, k, n, kb);
    
    // Выбор подходящего нижлежащего ядра
    gemm_4x4x8_int8_int32_v(C_padded, packed_A, packed_B, m_pad, n_pad, k_pad);

    // C_padded -> C (row-major)
    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < n; j++) {
            C[i * n + j] = C_padded[i * n_pad + j];
        }
    }

    free(packed_A);
    free(packed_B);
    free(C_padded);
}

// TODO: реализовать больше ядер
//       может лучше параметризовать размеры блока и типы данных

void gemm_4x4x8_int8_int32_v(
    int32_t *C,
    const int8_t *A_padded,
    const int8_t *B_padded,
    size_t m, size_t n, size_t k) // m, n, k здесь - padded размеры
{
    const size_t mb = 4;
    const size_t nb = 4;

    // Итерация по панелями по mb строк
    for (size_t i = 0; i < m; i += mb) {
        // Итерация панелями по nb столбцов
        for (size_t j = 0; j < n; j += nb) {
            // Получаем указатели на начало нужных панелей.
            const int8_t *A_panel = A_padded + (i / mb) * (k * mb);
            const int8_t *B_panel = B_padded + (j / nb) * (k * nb);

            // Указатель на целевой блок в матрице C.
            int32_t *C_dest_block = C + i * n + j;
            gepp_4x4x8_int8_int32_v(A_panel, B_panel, C_dest_block, k, n);
        }
    }
}

void gepp_4x4x8_int8_int32_v(
    const int8_t *A_panel,
    const int8_t *B_panel,
    int32_t *C_dest_block,
    size_t k,
    size_t n_stride
) {
    // Локальный аккумулятор на стеке. Компилятор постарается
    // разместить его в регистрах или L1 кэше.
    int32_t C_block[4 * 4];
    memset(C_block, 0, sizeof(C_block));

    // Цикл по внутреннему измерению k с шагом kb (здесь KR)
    for (size_t p = 0; p < k; p += 8) {
        // Вызываем ассемблерное микро-ядро для вычисления
        // C_block += A_micro_block * B_micro_block
        gebb_4x4x8_int8_int32_v(
            A_panel, // Указатель на текущий микро-блок A (4x8)
            B_panel, // Указатель на текущий микро-блок B (8x4)
            C_block  // Указатель на наш аккумулятор
        );

        // Перемещаем указатели на следующие микро-блоки
        A_panel += 4 * 8; // 4 * 8 = 32 байта
        B_panel += 8 * 4; // 8 * 4 = 32 байта
    }

    // После завершения цикла, копируем результат из локального
    // аккумулятора в итоговую матрицу C, соблюдая шаг n_stride.
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            C_dest_block[i * n_stride + j] = C_block[i * 4 + j];
        }
    }
}

