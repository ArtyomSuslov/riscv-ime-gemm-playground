#include "print_matrix.h"

void print_matrix_int8(const int8_t* mat, size_t rows, size_t cols, const char* name) {
    printf("%s (%zu x %zu):\n", name, rows, cols);
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            printf("%4d ", mat[i * cols + j]);
        }
        printf("\n");
    }
    printf("\n");
}

void print_matrix_int32(const int32_t* mat, size_t rows, size_t cols, const char* name) {
    printf("%s (%zu x %zu):\n", name, rows, cols);
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            printf("%4d ", mat[i * cols + j]);
        }
        printf("\n");
    }
    printf("\n");
}
