#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sched.h>
#include <unistd.h>
#include "kernel_ime/kernel.h"
#include "kernel_rvv/kernel_v.h"

// Эталонное умножение матриц int8->int32 (row-major)
void naive_gemm_int8_int32(
    int32_t *C,
    const int8_t *A,
    const int8_t *B,
    size_t m, size_t n, size_t k)
{
    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < n; j++) {
            int32_t sum = 0;
            for (size_t p = 0; p < k; p++) {
                sum += (int32_t)A[i*k + p] * (int32_t)B[p*n + j];
            }
            C[i*n + j] = sum;
        }
    }
}

// вычисление времени в секундах
double time_diff_sec(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main(int argc, char *argv[]) {
    // выполняем только на ИИ-ядрах (ядра с поддержкой инструкций IME)
    cpu_set_t mask;
    CPU_ZERO(&mask);
    
    CPU_SET(0, &mask);
    CPU_SET(1, &mask);
    CPU_SET(2, &mask);
    CPU_SET(3, &mask);

    pid_t pid = getpid();
    if (sched_setaffinity(pid, sizeof(cpu_set_t), &mask) == -1) {
        perror("sched_setaffinity");
        return 1;
    }

    srand(42);

    size_t m, n, k;

    for (m = 8; m <= 128; m += 1) {
        for (k = 8; k <= 128; k += 1) {
            n = m;

            printf("Size: m=%zu, n=%zu, k=%zu\n", m, n, k);

            int8_t *A = (int8_t*)malloc(m * k * sizeof(int8_t));
            int8_t *B = (int8_t*)malloc(k * n * sizeof(int8_t));

            int32_t *C_ref = (int32_t*)calloc(m * n, sizeof(int32_t));
            int32_t *C_gemm_v = (int32_t*)calloc(m * n, sizeof(int32_t));
            int32_t *C_gemm_ime = (int32_t*)calloc(m * n, sizeof(int32_t));

            if (!A || !B || !C_ref || !C_gemm_v || !C_gemm_ime) {
                fprintf(stderr, "Memory allocation failed\n");
                free(A); free(B); free(C_ref); free(C_gemm_ime); free(C_gemm_v);
                return 1;
            }

            // случайные значения для A и B
            for (size_t i = 0; i < m*k; i++) A[i] = rand() % 20 - 10;
            for (size_t i = 0; i < k*n; i++) B[i] = rand() % 20 - 10;

            struct timespec t1, t2;

            // замер времени эталонного GEMM
            clock_gettime(CLOCK_MONOTONIC, &t1);
            naive_gemm_int8_int32(C_ref, A, B, m, n, k);
            clock_gettime(CLOCK_MONOTONIC, &t2);
            double naive_time = time_diff_sec(t1, t2);

            // замер времени RVV GEMM
            clock_gettime(CLOCK_MONOTONIC, &t1);
            gemm_int8_int32_v(C_gemm_v, A, B, m, n, k);
            clock_gettime(CLOCK_MONOTONIC, &t2);
            double rvv_time = time_diff_sec(t1, t2);

            // замер времени IME GEMM
            clock_gettime(CLOCK_MONOTONIC, &t1);
            gemm_int8_int32(C_gemm_ime, A, B, m, n, k);
            clock_gettime(CLOCK_MONOTONIC, &t2);
            double ime_time = time_diff_sec(t1, t2);

            // Проверка на совпадение
            int correct_ime = 1;
            int correct_rvv = 1;
            int mismatches_printed = 0;
            for (size_t i = 0; i < m; i++) {
                for (size_t j = 0; j < n; ++j) {
                    // Проверка RVV
                    if (correct_rvv && C_gemm_v[i * n + j] != C_ref[i * n + j]) {
                        if (mismatches_printed < 10) {
                            printf("RVV Mismatch at (%zu, %zu): ref=%d, rvv=%d\n",
                                i, j, C_ref[i * n + j], C_gemm_v[i * n + j]);
                            mismatches_printed++;
                        }
                        correct_rvv = 0;
                    }
                    // Проверка IME
                    if (correct_ime && C_gemm_ime[i * n + j] != C_ref[i * n + j]) {
                        if (mismatches_printed < 10) {
                            printf("IME Mismatch at (%zu, %zu): ref=%d, ime=%d\n",
                                i, j, C_ref[i * n + j], C_gemm_ime[i * n + j]);
                            mismatches_printed++;
                        }
                        correct_ime = 0;
                    }
                }
            }

            if (correct_rvv) {
                printf("RVV Test: PASSED\n");
            } else {
                printf("RVV Test: FAILED\n");
            }
            if (correct_ime) {
                printf("IME Test: PASSED\n");
            } else {
                printf("IME Test: FAILED\n");
            }
            
            printf("Naive: %.6f s\n", naive_time);
            printf("RVV:   %.6f s (%.2fx speedup)\n", rvv_time, naive_time / rvv_time);
            printf("IME:   %.6f s (%.2fx speedup)\n", ime_time, naive_time / ime_time);
            printf("\n");

            free(A);
            free(B);
            free(C_gemm_ime);
            free(C_gemm_v);
            free(C_ref);
        }
    }

    return 0;
}