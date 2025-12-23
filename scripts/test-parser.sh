#!/bin/bash

# This script tests the G10 Assembler Tool's parser by running it
# on a set of test assembly files.

# Define the path to the G10 Assembler Tool executable
G10_ASM_TOOL="./build/bin/linux-debug/g10asm"

# Define the directories containing test assembly files
TEST_DIRS=("./examples/lexer" "./examples/parser")

# Loop through each test directory
for TEST_DIR in "${TEST_DIRS[@]}"; do
    # Loop through each `.s` and `.asm` file in the test directory
    for test_file in "$TEST_DIR"/*.s "$TEST_DIR"/*.asm; do
        # Check if the file exists to avoid errors if no files match
        if [[ -f "$test_file" ]]; then
            echo "Testing parser on file: $test_file"
            $G10_ASM_TOOL -s "$test_file" --parse-only
            # Check the exit status of the command
            if [[ $? -ne 0 ]]; then
                # Test files with 'error' in their name are expected to fail
                if [[ "$test_file" == *"error"* ]]; then
                    echo "Parser test failed, as expected, for file: $test_file"
                    continue
                fi

                echo "Parser test failed for file: $test_file"
                exit 1
            fi
        fi
    done
done

echo "Parser tests completed."
