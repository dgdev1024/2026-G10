#!/bin/bash

set -e
G10TMU_EXE="./build/bin/linux-debug/g10tmu"
G10_FILE="./build/bin/$1.g10"
G10_RAM_DUMP="./build/$(basename "${G10_FILE%.*}").ram"
echo "${@:2}"

# Run the G10 Testbed Emulator with the specified G10 program file.
# Use `-d` to dump RAM contents after execution to a file named `X.ram`, where
# `X` is the name of the G10 program file without its extension.
#
# Pass the remaining arguments to the G10TMU executable.
"$G10TMU_EXE" "$G10_FILE" -d "$G10_RAM_DUMP" "${@:2}"

# Hexdump the RAM dump file for inspection
echo "RAM dump from '$G10_RAM_DUMP':"
hexdump -C "$G10_RAM_DUMP"
echo ""
