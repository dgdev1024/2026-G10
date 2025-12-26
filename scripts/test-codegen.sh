#!/bin/bash

# This script tests the G10 Assembler Tool's code generation by running it
# on a set of test assembly files and checking the output object files.

# Define the path to the G10 Assembler Tool executable
G10_ASM_TOOL="./build/bin/linux-debug/g10asm"

# Define the directories containing test assembly files for code generation
TEST_DIRS=("./examples/lexer" "./examples/parser" "./examples/codegen")

# Define the directory to contain the generated object files
OUTPUT_DIR="./build/obj/test_codegen"
mkdir -p "$OUTPUT_DIR"

# Loop through each test directory
for TEST_DIR in "${TEST_DIRS[@]}"; do
    # Loop through each `.s` and `.asm` file in the test directory
    for test_file in "$TEST_DIR"/*.s "$TEST_DIR"/*.asm; do
        # Check if the file exists to avoid errors if no files match
        if [[ -f "$test_file" ]]; then
            echo "Testing code generation on file: $test_file"
            output_file="$OUTPUT_DIR/$(basename "${test_file%.*}.g10obj")"
            "$G10_ASM_TOOL" -s "$test_file" -o "$output_file"
            # Check the exit status of the command
            if [[ $? -ne 0 ]]; then
                # Test files with 'error' in their name are expected to fail
                if [[ "$test_file" == *"error"* ]]; then
                    echo "Code generation test failed, as expected, for file: $test_file"
                    continue
                fi

                echo "Code generation test failed for file: $test_file"
                exit 1
            fi

            # Hex dump the output object file for inspection
            echo "Hex dump of generated object file: $output_file"
            hexdump -C "$output_file"
            echo ""
        fi
    done
done

echo "Code generation tests completed."
