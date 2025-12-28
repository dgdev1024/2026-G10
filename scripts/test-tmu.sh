#!/bin/bash

# Test the G10 linker by linking sample programs and inspecting the output 
# executable files.

# Define the paths to the G10 Assembler and Linker executables
G10_ASM_TOOL="./build/bin/linux-debug/g10asm"
G10_LINKER_TOOL="./build/bin/linux-debug/g10link"

# Define the directory containing test assembly files for linking
TEST_DIR="./examples/g10tmu"

# Define the directory to contain the object files generated for linking
OBJ_OUTPUT_DIR="./build/obj"
mkdir -p "$OBJ_OUTPUT_DIR"

# Define the directory to contain the final linked executables
EXE_OUTPUT_DIR="./build/bin"
mkdir -p "$EXE_OUTPUT_DIR"

# Loop through each entry in the test directory
# - If the entry is a directory, then there are multiple files to assemble
#   and link together into one executable. Create a directory in the output
#   locations to mirror the structure.
# - If the entry is a single file, then just assemble and link that file.
for entry in "$TEST_DIR"/*; do
    if [[ -d "$entry" ]]; then
        echo "Linking multiple files in directory: $entry"
        obj_files=()

        # Create mirrored output directories
        dir_name=$(basename "$entry")
        OBJ_DIR="$OBJ_OUTPUT_DIR/$dir_name"
        EXE_DIR="$EXE_OUTPUT_DIR/$dir_name"
        mkdir -p "$OBJ_DIR"
        mkdir -p "$EXE_DIR"

        # Assemble each `.s` and `.asm` file in the directory
        for asm_file in "$entry"/*.s "$entry"/*.asm; do
            if [[ -f "$asm_file" ]]; then
                echo "Assembling file: $asm_file"
                obj_file="$OBJ_DIR/$(basename "${asm_file%.*}.g10obj")"

                "$G10_ASM_TOOL" -s "$asm_file" -o "$obj_file"
                if [[ $? -ne 0 ]]; then
                    echo "Assembly failed for file: $asm_file"
                    exit 1
                fi

                obj_files+=("$obj_file")
            fi
        done

        # Link the assembled object files into one executable
        exe_file="$EXE_DIR/$(basename "$entry").g10"

        "$G10_LINKER_TOOL" "${obj_files[@]}" -o "$exe_file"
        if [[ $? -ne 0 ]]; then
            echo "Linking failed for directory: $entry"
            exit 1
        fi
    elif [[ -f "$entry" ]]; then
        echo "Linking single file: $entry"
        obj_file="$OBJ_OUTPUT_DIR/$(basename "${entry%.*}.g10obj")"

        "$G10_ASM_TOOL" -s "$entry" -o "$obj_file"
        if [[ $? -ne 0 ]]; then
            echo "Assembly failed for file: $entry"
            exit 1
        fi

        exe_file="$EXE_OUTPUT_DIR/$(basename "${entry%.*}.g10")"

        "$G10_LINKER_TOOL" "$obj_file" -o "$exe_file"
        if [[ $? -ne 0 ]]; then
            echo "Linking failed for file: $entry"
            exit 1
        fi
    fi
done

echo "Testbed example programs assembled and linked successfully."