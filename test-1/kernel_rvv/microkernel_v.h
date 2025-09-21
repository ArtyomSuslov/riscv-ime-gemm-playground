#ifndef MICROKERNEL_VECTOR_H
#define MICROKERNEL_VECTOR_H

#include <stdint.h>
#include <stddef.h>

void gebb_4x4x8_int8_int32_v(
    const int8_t *A_panel,
    const int8_t *B_panel,
    int32_t *C_dest_block
);

#endif // MICROKERNEL_VECTOR_H