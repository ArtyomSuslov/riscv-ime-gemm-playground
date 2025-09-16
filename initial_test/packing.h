#ifndef PACKING_H
#define PACKING_H

#include <stdint.h>
#include <stddef.h>

void pack_A_4xK_rowmajor(int8_t* packed_a, const int8_t* a, size_t lda, size_t k);
void pack_B_Kx4_colmajor(int8_t* packed_b, const int8_t* b, size_t ldb, size_t k);

#endif // MICROKERNEL_H