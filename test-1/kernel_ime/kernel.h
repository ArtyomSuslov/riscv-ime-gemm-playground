#ifndef KERNEL_H
#define KERNEL_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "microkernel.h"
#include "../packing/packing.h"
#include "../extra_funcs/print_matrix.h"

void gepp_4x4xk(
    const int8_t *A_panel,
    const int8_t *B_panel,
    int32_t *C_block,
    size_t k);

void gemm_4x4x8_int8_int32(
    int32_t *C,
    const int8_t *A_padded,
    const int8_t *B_padded,
    size_t m, size_t n, size_t k);

void gemm_int8_int32(
    int32_t *C,
    int8_t *A,
    int8_t *B,
    size_t m,
    size_t n,
    size_t k,
    bool pre_packed_B);

#endif // KERNEL_H
