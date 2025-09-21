.global gepp_microkernel_4x4_int8_int32

gepp_microkernel_4x4_int8_int32:
    # a0: int32_t*      c,  
    # a1: const int8_t* a,  
    # a2: const int8_t* b,  
    # a3: size_t        k
    
    # VLEN=256, SEW=8 (4x4x8)
    li t0, 256
    vsetvli x0, t0, e8, m1, ta, ma

    # Загрузка/обнуление аккумулятора C (4x4)
    vmv.v.i v0, 0
    vmv.v.i v1, 0

.L_main_loop:
    # Тело цикла по K
    # На каждой итерации обрабатываем k=8.
    
    # Загружаем микро-панель A (4x8, 32 байта) в v8.
    vle8.v v8, (a1)

    # Загружаем микро-панель B (8x4, 32 байта) в v10.
    vle8.v v10, (a2)

    # vmadot vd, vs1, vs2 -> C(4x4) += A(4x8) * B(8x4)
    vmadot v0, v8, v10
    
    # Сдвигаем указатели для следующей итерации
    addi a1, a1, 32 # A(4x8) -> 4*8*1 = 32 байта
    addi a2, a2, 32 # B(8x4) -> 8*4*1 = 32 байта
    
    # Уменьшаем счетчик k и повторяем цикл
    addi a3, a3, -8
    bnez a3, .L_main_loop

    # Сохранение результата из v0, v1 обратно в C
    li t0, 8
    vsetvli x0, t0, e32, m1, ta, ma

    vse32.v v0, (a0)
    addi t1, a0, 32
    vse32.v v1, (t1)
    
    ret