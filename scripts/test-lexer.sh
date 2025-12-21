#!/bin/bash

# This script tests the G10 Assembler Tool's lexical analyzer by running it
# on a set of test assembly files.

# Define the path to the G10 Assembler Tool executable
G10_ASM_TOOL="./build/bin/linux-debug/g10asm"

# Define the directory containing test assembly files
TEST_DIR="./examples/lexer"

# Loop through each `.s` and `.asm` file in the test directory
for test_file in "$TEST_DIR"/*.s "$TEST_DIR"/*.asm; do
    # Check if the file exists to avoid errors if no files match
    if [[ -f "$test_file" ]]; then
        echo "Testing lexer on file: $test_file"
        
        # Run the G10 Assembler Tool with the `--lex-only` option
        "$G10_ASM_TOOL" --lex-only -s "$test_file"

        # Check the exit status of the command
        if [[ $? -ne 0 ]]; then
            echo "Lexer test failed for file: $test_file"
            exit 1
        fi
    fi
done

echo "Lexer tests completed."
