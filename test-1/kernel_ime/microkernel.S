#
# void gepp_4x4x8_int8_int32(
#     const int8_t *A_panel,        // a0
#     const int8_t *B_panel,        // a1
#     int32_t *C_dest_block,        // a2
#     size_t k,                     // a3
#     size_t n_stride               // a4
# );
#
# Ядро вычисляет C_block(4x4) = A_panel(4xk) * B_panel(kx4)
# и сохраняет результат в C.
#
.global gepp_4x4x8_int8_int32
.align 2

gepp_4x4x8_int8_int32:
    # Пролог
    addi sp, sp, -40
    sd   ra, 32(sp)
    sd   s0, 24(sp)
    sd   s1, 16(sp)
    sd   s2, 8(sp)
    sd   s3, 0(sp)

    # Сохраняем аргументы в s-регистры для удобства
    mv   s0, a0  # s0 = A_panel
    mv   s1, a1  # s1 = B_panel
    mv   s2, a2  # s2 = C_dest_block
                 # a3 (k) остается в a3
    mv   s3, a4  # s3 = n_stride

    # Инициализация аккумулятора C_block (4x4, int32_t)
    vsetvli t0, x0, e32, m2, ta, ma
    vmv.v.i v24, 0

    # Цикл по внутреннему измерению k
    # p - счетчик цикла
    li   a5, 0  # p = 0

loop_k:
    # Внутри цикла мы работаем с блоками 4x8 и 8x4
    vsetvli t0, x0, e8, m1, ta, ma

    vle8.v v8, (s0)
    vle8.v v16, (s1)

    # C_acc += A_block * B_block
    vmadot v24, v8, v16 # v24 - четный

    # Перемещаем указатели на следующие микро-блоки
    addi s0, s0, 32
    addi s1, s1, 32
    
    # Увеличиваем счетчик и проверяем условие выхода из цикла
    addi a5, a5, 8
    blt  a5, a3, loop_k # if (p < k) goto loop_k

    # Сохранение результата из аккумулятора в матрицу C
    # Результат (64 байта) находится в v24, v25.
    # Нам нужно сохранить его в C_dest_block с шагом n_stride
    
    # Проще всего сначала сохранить на стек, а потом скопировать.
    # Выделяем 64 байта на стеке
    addi sp, sp, -64
    
    # Сохраняем v24, v25 на стек
    vsetvli t0, x0, e32, m2, ta, ma
    vse32.v v24, (sp)

    # Теперь копируем со стека в C с правильным шагом
    # s3 = n_stride (в байтах: n_pad * 4)
    slli s3, s3, 2 # n_stride в байтах
    
    # Загрузка и сохранение по 2 слова (16 байт) за итерацию
    mv   a0, sp    # src = &stack[0]
    mv   a1, s2    # dst = C_dest_block
    
    # Копируем 4 строки
    ld   t0, 0(a0) # row 0
    ld   t1, 8(a0)
    sd   t0, 0(a1)
    sd   t1, 8(a1)
    add  a0, a0, 16
    add  a1, a1, s3

    ld   t0, 0(a0) # row 1
    ld   t1, 8(a0)
    sd   t0, 0(a1)
    sd   t1, 8(a1)
    add  a0, a0, 16
    add  a1, a1, s3

    ld   t0, 0(a0) # row 2
    ld   t1, 8(a0)
    sd   t0, 0(a1)
    sd   t1, 8(a1)
    add  a0, a0, 16
    add  a1, a1, s3

    ld   t0, 0(a0) # row 3
    ld   t1, 8(a0)
    sd   t0, 0(a1)
    sd   t1, 8(a1)

    # Восстановление стека и регистров
    addi sp, sp, 64  # Освобождаем место под C_block
    
    ld   ra, 32(sp)
    ld   s0, 24(sp)
    ld   s1, 16(sp)
    ld   s2, 8(sp)
    ld   s3, 0(sp)
    addi sp, sp, 40

    ret