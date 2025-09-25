#ifndef PRINT_MATRIX_H
#define PRINT_MATRIX_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

void print_matrix_int8(const int8_t* mat, size_t rows, size_t cols, const char* name);
void print_matrix_int32(const int32_t* mat, size_t rows, size_t cols, const char* name);

#endif // PRINT_MATRIX_H
