#ifndef PACKING_H
#define PACKING_H

#include <stdint.h>
#include <stddef.h>

void pack_A_4xK(int8_t* packed_a, const int8_t* a,
                size_t n, size_t k, size_t block_size);


void pack_B_Kx4(int8_t* packed_b, const int8_t* b,
                size_t k, size_t m, size_t block_size);

#endif // PACKING_H