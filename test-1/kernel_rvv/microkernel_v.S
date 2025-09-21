#
# void gebb_4x4x8_int8_int32_v(
#     const int8_t *A_block,    // a0
#     const int8_t *B_block,    // a1
#     int32_t *C_block          // a2
# );
#
# Микро-ядро вычисляет C_block(4x4) += A_block(4x8) * B_block(8x4).
# Эта функция является "листовой" (leaf function), она не вызывает другие
# функции и использует только caller-saved регистры (a*, t*),
# поэтому ей не требуются пролог и эпилог для сохранения регистров.
#
.global gebb_4x4x8_int8_int32_v
.align 2

gebb_4x4x8_int8_int32_v:
    # Шаг 1: Загрузка аккумулятора C (int32) в векторные регистры v24-v27
    li      t5, 4
    vsetvli x0, t5, e32, m1, ta, ma # vl=4, SEW=32 для работы с аккумулятором
    vle32.v v24, (a2)             # Загрузка C_row[0]
    addi    a2, a2, 16
    vle32.v v25, (a2)             # Загрузка C_row[1]
    addi    a2, a2, 16
    vle32.v v26, (a2)             # Загрузка C_row[2]
    addi    a2, a2, 16
    vle32.v v27, (a2)             # Загрузка C_row[3]

    # Шаг 2: Цикл по внутреннему измерению k (развернут)
    li      t4, 8                 # Stride (шаг) для загрузки строк из B = 8 байт

    # p = 0
    lb        t0, 0(a0); lb t1, 8(a0); lb t2, 16(a0); lb t3, 24(a0)
    vlse8.v   v8, (a1), t4
    vsext.vf4 v16, v8
    vmacc.vx v24, t0, v16; vmacc.vx v25, t1, v16; vmacc.vx v26, t2, v16; vmacc.vx v27, t3, v16
    
    # p = 1
    lb        t0, 1(a0); lb t1, 9(a0); lb t2, 17(a0); lb t3, 25(a0)
    addi      t6, a1, 1
    vlse8.v   v8, (t6), t4
    vsext.vf4 v16, v8
    vmacc.vx v24, t0, v16; vmacc.vx v25, t1, v16; vmacc.vx v26, t2, v16; vmacc.vx v27, t3, v16
    
    # p = 2
    lb        t0, 2(a0); lb t1, 10(a0); lb t2, 18(a0); lb t3, 26(a0)
    addi      t6, a1, 2
    vlse8.v   v8, (t6), t4
    vsext.vf4 v16, v8
    vmacc.vx v24, t0, v16; vmacc.vx v25, t1, v16; vmacc.vx v26, t2, v16; vmacc.vx v27, t3, v16
    
    # p = 3
    lb        t0, 3(a0); lb t1, 11(a0); lb t2, 19(a0); lb t3, 27(a0)
    addi      t6, a1, 3
    vlse8.v   v8, (t6), t4
    vsext.vf4 v16, v8
    vmacc.vx v24, t0, v16; vmacc.vx v25, t1, v16; vmacc.vx v26, t2, v16; vmacc.vx v27, t3, v16
    
    # p = 4
    lb        t0, 4(a0); lb t1, 12(a0); lb t2, 20(a0); lb t3, 28(a0)
    addi      t6, a1, 4
    vlse8.v   v8, (t6), t4
    vsext.vf4 v16, v8
    vmacc.vx v24, t0, v16; vmacc.vx v25, t1, v16; vmacc.vx v26, t2, v16; vmacc.vx v27, t3, v16
    
    # p = 5
    lb        t0, 5(a0); lb t1, 13(a0); lb t2, 21(a0); lb t3, 29(a0)
    addi      t6, a1, 5
    vlse8.v   v8, (t6), t4
    vsext.vf4 v16, v8
    vmacc.vx v24, t0, v16; vmacc.vx v25, t1, v16; vmacc.vx v26, t2, v16; vmacc.vx v27, t3, v16
    
    # p = 6
    lb        t0, 6(a0); lb t1, 14(a0); lb t2, 22(a0); lb t3, 30(a0)
    addi      t6, a1, 6
    vlse8.v   v8, (t6), t4
    vsext.vf4 v16, v8
    vmacc.vx v24, t0, v16; vmacc.vx v25, t1, v16; vmacc.vx v26, t2, v16; vmacc.vx v27, t3, v16
    
    # p = 7
    lb        t0, 7(a0); lb t1, 15(a0); lb t2, 23(a0); lb t3, 31(a0)
    addi      t6, a1, 7
    vlse8.v   v8, (t6), t4
    vsext.vf4 v16, v8
    vmacc.vx v24, t0, v16; vmacc.vx v25, t1, v16; vmacc.vx v26, t2, v16; vmacc.vx v27, t3, v16

    # Шаг 3: Сохранение результата из аккумулятора в матрицу C
    add     a2, a2, -48          # Восстанавливаем указатель на начало C_block
    vse32.v v24, (a2); addi a2, a2, 16
    vse32.v v25, (a2); addi a2, a2, 16
    vse32.v v26, (a2); addi a2, a2, 16
    vse32.v v27, (a2)

    ret