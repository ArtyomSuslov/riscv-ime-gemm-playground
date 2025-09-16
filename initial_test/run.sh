#!/bin/bash
set -e

if [ $# -lt 2 ]; then
    echo "Использование: $0 output_binary source1.[c|s] [source2.[c|s] ...]"
    exit 1
fi

OUTPUT=$1
shift

GCC_BIN="$SPACEMIT_SDK_PATH/spacemit-gcc/bin/riscv64-unknown-linux-gnu-gcc"
QEMU_BIN="$SPACEMIT_SDK_PATH/spacemit-qemu/bin/qemu-riscv64"
SYSROOT="$SPACEMIT_SDK_PATH/spacemit-gcc/sysroot"

echo "[*] Компиляция: $@ -> $OUTPUT"
"$GCC_BIN" -static -march=rv64gcv -mabi=lp64 -o "$OUTPUT" "$@"

echo "[+] Сборка успешна: $OUTPUT"

if [ -f "$OUTPUT" ]; then
    echo "[*] Запуск через QEMU:"
    "$QEMU_BIN" -L "$SYSROOT" "$OUTPUT"
fi
