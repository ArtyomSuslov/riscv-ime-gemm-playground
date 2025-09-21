#ifndef MICROKERNEL_H
#define MICROKERNEL_H

#include <stdint.h>
#include <stddef.h>

void gepp_4x4x8_int8_int32(
    const int8_t *A_panel,
    const int8_t *B_panel,
    int32_t *C_dest_block,
    size_t k,
    size_t n_stride
);

#endif // MICROKERNEL_H