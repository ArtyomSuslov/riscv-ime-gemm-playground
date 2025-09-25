#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <sched.h>
#include <unistd.h>
#include <string.h>
#include "kernel_ime/kernel.h"
#include "kernel_rvv/kernel_v.h"
#include "packing/packing.h"

#define NUM_RUNS 5
#define SPARSE_THRESHOLD 9 // 90% нулей

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
                if (sum > (INT32_MAX / 2)) {
                    fprintf(stderr, "Overflow warning at (%zu, %zu, %zu)\n", i, j, p);
                }
            }
            C[i*n + j] = sum;
        }
    }
}

// вычисление времени в секундах
double time_diff_sec(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

// Фиксированный тест для проверки корректности
void test_fixed_gemm() {
    int8_t A[4*4] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    int8_t B[4*4] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    int32_t C_ref[4*4] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    int32_t C_v[4*4], C_ime[4*4];

    // Тест с packing
    gemm_int8_int32_v(C_v, A, B, 4, 4, 4, false);
    gemm_int8_int32(C_ime, A, B, 4, 4, 4, false);
    int correct_v = 1, correct_ime = 1;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (C_v[i*4 + j] != C_ref[i*4 + j]) correct_v = 0;
            if (C_ime[i*4 + j] != C_ref[i*4 + j]) correct_ime = 0;
        }
    }
    printf("Fixed Test (with packing): RVV %s, IME %s\n", 
        correct_v ? "PASSED" : "FAILED", 
        correct_ime ? "PASSED" : "FAILED"
    );

    // Тест pre_packed
    int8_t packed_B[4*4];
    pack_B_Kx4(packed_B, B, 4, 4, 8);
    gemm_int8_int32_v(C_v, A, packed_B, 4, 4, 4, true);
    gemm_int8_int32(C_ime, A, packed_B, 4, 4, 4, true);
    correct_v = 1; correct_ime = 1;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (C_v[i*4 + j] != C_ref[i*4 + j]) correct_v = 0;
            if (C_ime[i*4 + j] != C_ref[i*4 + j]) correct_ime = 0;
        }
    }
    printf("Fixed Test (pre_packed): RVV %s, IME %s\n", 
        correct_v ? "PASSED" : "FAILED", 
        correct_ime ? "PASSED" : "FAILED"
    );
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

    // Выполняем фиксированный тест
    #ifdef DEBUG
    test_fixed_gemm();
    #endif

    size_t m, n, k;

    for (m = 128; m <= 1024; m += 128) {
        // for (n = 8; n <= 128; n += 8) {
            for (k = 128; k <= 1024; k += 128) {
                n = m;
                const char *modes[] = {"with_packing", "pre_packed"};  
                for (int mode_idx = 0; mode_idx < 2; mode_idx++) {  

                    printf("Size: m=%zu, n=%zu, k=%zu\n", m, n, k);
                    printf("Mode: %s\n", modes[mode_idx]);

                    int8_t *A = (int8_t*)malloc(m * k * sizeof(int8_t));
                    int8_t *B = (int8_t*)malloc(k * n * sizeof(int8_t));
                    int8_t *packed_B_pre = NULL;  

                    int32_t *C_ref = (int32_t*)calloc(m * n, sizeof(int32_t));
                    int32_t *C_gemm_v = (int32_t*)calloc(m * n, sizeof(int32_t));
                    int32_t *C_gemm_ime = (int32_t*)calloc(m * n, sizeof(int32_t));

                    if (!A || !B || !C_ref || !C_gemm_v || !C_gemm_ime) {
                        fprintf(stderr, "Memory allocation failed\n");
                        free(A); free(B); free(C_ref); free(C_gemm_ime); free(C_gemm_v);
                        return 1;
                    }

                    // случайные значения для A и B с sparse mode для B
                    for (size_t i = 0; i < m*k; i++) A[i] = rand() % 256 - 128;
                    for (size_t i = 0; i < k*n; i++) B[i] = (rand() % 10 < SPARSE_THRESHOLD) ? 0 : (rand() % 256 - 128);

                    // для pre_packed упакуем B заранее
                    if (strcmp(modes[mode_idx], "pre_packed") == 0) {
                        size_t kb = 8;
                        size_t n_pad = ((n + 4 - 1) / 4) * 4;
                        size_t k_pad = ((k + kb - 1) / kb) * kb;
                        size_t size_packed_B = k_pad * n_pad;
                        packed_B_pre = (int8_t*)malloc(size_packed_B * sizeof(int8_t));
                        if (!packed_B_pre) {
                            fprintf(stderr, "Packed B alloc failed\n");
                        }
                        pack_B_Kx4(packed_B_pre, B, k, n, kb);
                    }

                    struct timespec t1, t2;
                    double packing_time_ime = 0.0, packing_time_v = 0.0;

                    double times_naive[NUM_RUNS], times_rvv[NUM_RUNS], times_ime[NUM_RUNS];

                    // Warm-up
                    naive_gemm_int8_int32(C_ref, A, B, m, n, k);
                    gemm_int8_int32_v(C_gemm_v, A, B, m, n, k, strcmp(modes[mode_idx], "pre_packed") == 0);
                    gemm_int8_int32(C_gemm_ime, A, B, m, n, k, strcmp(modes[mode_idx], "pre_packed") == 0);

                    // замер времени эталонного GEMM
                    for (int run = 0; run < NUM_RUNS; run++) {
                        clock_gettime(CLOCK_MONOTONIC, &t1);
                        naive_gemm_int8_int32(C_ref, A, B, m, n, k);
                        clock_gettime(CLOCK_MONOTONIC, &t2);
                        times_naive[run] = time_diff_sec(t1, t2);
                    }
                    double naive_time = 0.0;
                    for (int run = 0; run < NUM_RUNS; run++) naive_time += times_naive[run];
                    naive_time /= NUM_RUNS;

                    // замер времени RVV GEMM
                    for (int run = 0; run < NUM_RUNS; run++) {
                        clock_gettime(CLOCK_MONOTONIC, &t1);
                        if (strcmp(modes[mode_idx], "pre_packed") == 0) {
                            gemm_int8_int32_v(C_gemm_v, A, packed_B_pre, m, n, k, true);
                        } else {
                            gemm_int8_int32_v(C_gemm_v, A, B, m, n, k, false);
                        }
                        clock_gettime(CLOCK_MONOTONIC, &t2);
                        times_rvv[run] = time_diff_sec(t1, t2);
                    }
                    double rvv_time = 0.0;
                    for (int run = 0; run < NUM_RUNS; run++) rvv_time += times_rvv[run];
                    rvv_time /= NUM_RUNS;

                    // замер времени IME GEMM
                    for (int run = 0; run < NUM_RUNS; run++) {
                        clock_gettime(CLOCK_MONOTONIC, &t1);
                        if (strcmp(modes[mode_idx], "pre_packed") == 0) {
                            gemm_int8_int32(C_gemm_ime, A, packed_B_pre, m, n, k, true);
                        } else {
                            gemm_int8_int32(C_gemm_ime, A, B, m, n, k, false);
                        }
                        clock_gettime(CLOCK_MONOTONIC, &t2);
                        times_ime[run] = time_diff_sec(t1, t2);
                    }
                    double ime_time = 0.0;
                    for (int run = 0; run < NUM_RUNS; run++) ime_time += times_ime[run];
                    ime_time /= NUM_RUNS;

                    #ifdef DEBUG
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
                    #endif
                    
                    double gops_naive = (2.0 * m * n * k) / (naive_time * 1e9);
                    double gops_rvv = (2.0 * m * n * k) / (rvv_time * 1e9);
                    double gops_ime = (2.0 * m * n * k) / (ime_time * 1e9);
                    printf("Naive: %.6f s (%.2f GOPS)\n", naive_time, gops_naive);
                    printf("RVV:   %.6f s (%.2f GOPS, %.2fx speedup)\n", rvv_time, gops_rvv, naive_time / rvv_time);
                    printf("IME:   %.6f s (%.2f GOPS, %.2fx speedup)\n", ime_time, gops_ime, naive_time / ime_time);
                    printf("\n");

                    free(A);
                    free(B);
                    free(C_gemm_ime);
                    free(C_gemm_v);
                    free(C_ref);
                    free(packed_B_pre);
                }
            }
        // }
    }

    return 0;
}