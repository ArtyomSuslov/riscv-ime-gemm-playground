#!/bin/bash
set -e

DEBUG_PORT=""

# Parse optional -g <port>
while [[ $# -gt 0 ]]; do
    case "$1" in
        -g)
            if [[ -z "$2" ]]; then
                echo "Ошибка: флаг -g требует указания порта"
                exit 1
            fi
            DEBUG_PORT="$2"
            shift 2
            ;;
        *)
            break
            ;;
    esac
done

if [ $# -lt 2 ]; then
    echo "Использование: $0 [-g port] output_binary source1.[c|s] [source2.[c|s] ...]"
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
    QEMU_CMD=("$QEMU_BIN" -L "$SYSROOT")
    if [[ -n "$DEBUG_PORT" ]]; then
        QEMU_CMD+=("-g" "$DEBUG_PORT")
        echo "[*] Включен режим GDB на порту $DEBUG_PORT (остановка до старта)"
    fi
    QEMU_CMD+=("$OUTPUT")
    "${QEMU_CMD[@]}"
fi
