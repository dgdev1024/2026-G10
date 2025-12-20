#!/bin/bash
# =============================================================================
# G10 Linker Test Suite
# =============================================================================
# This script assembles and links all test examples to verify the toolchain.
# =============================================================================

set -e  # Exit on first error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
ASM="$PROJECT_ROOT/build/bin/linux-debug/g10-asm"
LINK="$PROJECT_ROOT/build/bin/linux-debug/g10-link"
EXAMPLES="$PROJECT_ROOT/examples/linker"
OUTPUT_DIR="/tmp/g10-linker-tests"

# Counters
PASS=0
FAIL=0
TOTAL=0

# Create output directory
mkdir -p "$OUTPUT_DIR"

echo -e "${BLUE}=============================================${NC}"
echo -e "${BLUE}  G10 Linker Test Suite${NC}"
echo -e "${BLUE}=============================================${NC}"
echo ""

# Function to run a single-file test
test_single() {
    local name="$1"
    local asm_file="$2"
    
    TOTAL=$((TOTAL + 1))
    echo -n "Test $TOTAL: $name... "
    
    local obj_file="$OUTPUT_DIR/${name}.g10obj"
    local exe_file="$OUTPUT_DIR/${name}.g10"
    
    # Assemble
    if ! $ASM -s "$asm_file" -o "$obj_file" 2>/dev/null; then
        echo -e "${RED}FAIL${NC} (assembly error)"
        FAIL=$((FAIL + 1))
        return 1
    fi
    
    # Link
    if ! $LINK -o "$exe_file" "$obj_file" 2>/dev/null; then
        echo -e "${RED}FAIL${NC} (link error)"
        FAIL=$((FAIL + 1))
        return 1
    fi
    
    # Verify output exists and has reasonable size
    if [ ! -f "$exe_file" ] || [ ! -s "$exe_file" ]; then
        echo -e "${RED}FAIL${NC} (no output)"
        FAIL=$((FAIL + 1))
        return 1
    fi
    
    local size=$(stat -c%s "$exe_file" 2>/dev/null || stat -f%z "$exe_file" 2>/dev/null)
    echo -e "${GREEN}PASS${NC} (${size} bytes)"
    PASS=$((PASS + 1))
    return 0
}

# Function to run a multi-file test
test_multi() {
    local name="$1"
    shift
    local asm_files=("$@")
    
    TOTAL=$((TOTAL + 1))
    echo -n "Test $TOTAL: $name (multi-file)... "
    
    local obj_files=()
    local exe_file="$OUTPUT_DIR/${name}.g10"
    
    # Assemble each file
    for asm_file in "${asm_files[@]}"; do
        local base=$(basename "$asm_file" .asm)
        local obj_file="$OUTPUT_DIR/${base}.g10obj"
        
        if ! $ASM -s "$asm_file" -o "$obj_file" 2>/dev/null; then
            echo -e "${RED}FAIL${NC} (assembly error: $base)"
            FAIL=$((FAIL + 1))
            return 1
        fi
        
        obj_files+=("$obj_file")
    done
    
    # Link all object files
    if ! $LINK -o "$exe_file" "${obj_files[@]}" 2>/dev/null; then
        echo -e "${RED}FAIL${NC} (link error)"
        FAIL=$((FAIL + 1))
        return 1
    fi
    
    # Verify output exists and has reasonable size
    if [ ! -f "$exe_file" ] || [ ! -s "$exe_file" ]; then
        echo -e "${RED}FAIL${NC} (no output)"
        FAIL=$((FAIL + 1))
        return 1
    fi
    
    local size=$(stat -c%s "$exe_file" 2>/dev/null || stat -f%z "$exe_file" 2>/dev/null)
    echo -e "${GREEN}PASS${NC} (${size} bytes)"
    PASS=$((PASS + 1))
    return 0
}

echo -e "${YELLOW}--- Single-File Tests ---${NC}"
echo ""

# Single-file tests
test_single "simple-counter" "$EXAMPLES/1-simple-counter.asm"
test_single "main-entry" "$EXAMPLES/2-main-entry.asm"
test_single "arithmetic" "$EXAMPLES/3-arithmetic.asm"
test_single "branching" "$EXAMPLES/4-branching.asm"
test_single "stack-ops" "$EXAMPLES/5-stack-ops.asm"
test_single "bitwise" "$EXAMPLES/6-bitwise.asm"
test_single "strings" "$EXAMPLES/8-strings.asm"
test_single "register-moves" "$EXAMPLES/9-register-moves.asm"
test_single "cpu-control" "$EXAMPLES/10-cpu-control.asm"
test_single "rotate" "$EXAMPLES/11-rotate.asm"
test_single "swap" "$EXAMPLES/12-swap.asm"
test_single "quickram-io" "$EXAMPLES/14-quickram-io.asm"
test_single "conditions" "$EXAMPLES/15-conditions.asm"

echo ""
echo -e "${YELLOW}--- Multi-File Tests ---${NC}"
echo ""

# Multi-file tests
test_multi "math-project" \
    "$EXAMPLES/7a-main.asm" \
    "$EXAMPLES/7b-math-lib.asm"

test_multi "complex-project" \
    "$EXAMPLES/13a-main.asm" \
    "$EXAMPLES/13b-utils.asm" \
    "$EXAMPLES/13c-data.asm"

echo ""
echo -e "${BLUE}=============================================${NC}"
echo -e "${BLUE}  Test Results${NC}"
echo -e "${BLUE}=============================================${NC}"
echo ""
echo -e "Total:  $TOTAL"
echo -e "Passed: ${GREEN}$PASS${NC}"
echo -e "Failed: ${RED}$FAIL${NC}"
echo ""

if [ $FAIL -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed!${NC}"
    exit 1
fi
