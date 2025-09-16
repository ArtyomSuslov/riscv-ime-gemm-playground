#!/bin/bash

if [ -z "$1" ]; then
    echo "Использование: $0 /путь/до/spacemit_sdk"
    exit 1
fi

export SPACEMIT_SDK_PATH="$1"
echo "SPACEMIT_SDK_PATH установлен в: $SPACEMIT_SDK_PATH"
