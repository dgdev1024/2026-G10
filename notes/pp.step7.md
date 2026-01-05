# Step 7: Loop Assembly Blocks

This document provides detailed step-by-step instructions for implementing
loop assembly blocks in the G10 Assembler's preprocessor.

## Overview

Loop assembly blocks allow code to be repeated multiple times during
preprocessing. The G10 preprocessor supports three types of loops:

| Directive | Description |
|-----------|-------------|
| `.repeat <count> [, <var>]` / `.rept` | Repeat block `count` times |
| `.for <var>, <start>, <end> [, <step>]` | For loop with counter variable |
| `.while <condition> [, <var>]` | While loop with condition |

Control flow within loops:

| Directive | Description |
|-----------|-------------|
| `.continue` | Skip to next iteration |
| `.break` | Exit the loop entirely |

## References

- [G10 Preprocessor Language Specification](../docs/g10pp.specification.md)
- [G10 Code Style Guide](../docs/g10.code_style.md)
- [Preprocessor Header](../projects/g10asm/preprocessor.hpp)
- [Preprocessor Implementation](../projects/g10asm/preprocessor.cpp)
- [Step 6: Conditional Assembly](./pp.step6.md)

## Implementation Design

### Loop State Structure

Since loops can be nested, we need a stack to track the state of each loop.
Each entry contains:

```cpp
enum class loop_type
{
    repeat,     // .repeat/.rept loop
    for_loop,   // .for loop
    while_loop  // .while loop
};

struct loop_state
{
    loop_type type;                     // Type of loop
    std::string variable_name;          // Loop variable name (optional for repeat)
    std::int64_t current_value;         // Current value of loop variable
    std::int64_t end_value;             // End value (for .for loops)
    std::int64_t step_value;            // Step value (for .for loops)
    std::size_t iteration_count;        // Current iteration (0-based)
    std::size_t max_iterations;         // Maximum iterations (for .repeat)
    std::size_t body_start_index;       // Token index where loop body starts
    std::vector<token> condition_tokens; // Condition tokens (for .while)
    std::string source_file;            // Source file for error reporting
    std::size_t source_line;            // Source line for error reporting
    bool should_break;                  // Flag to exit loop
    bool should_continue;               // Flag to skip to next iteration
};
```

### Loop Execution Strategy

Loops are implemented by:

1. **Collecting loop body tokens**: When a loop directive is encountered,
   collect all tokens until the matching end directive (tracking nesting).

2. **Creating a temporary macro**: The loop variable is defined as a temporary
   macro that can be referenced within the loop body.

3. **Re-processing tokens**: For each iteration, re-inject the loop body
   tokens into the token stream for processing.

### Alternative Strategy: Token Replay

A simpler approach for this preprocessor:

1. When encountering a loop start, record the current token index
2. Collect loop body tokens into a vector
3. For each iteration:
   - Define/update the loop variable macro
   - Process the collected tokens
   - Check for `.continue` or `.break`
4. After all iterations, remove the loop variable macro

## Loop Specifications

### `.repeat` / `.rept` Loop

```asm
.repeat <count> [, <var>]
    ; Loop body - repeated <count> times
    ; If <var> is provided, it contains 0, 1, 2, ..., count-1
.endrepeat
```

- `<count>` is evaluated once at loop start
- If `<count>` <= 0, the loop body is skipped entirely
- `<var>` is optional; if provided, it's a local variable

### `.for` Loop

```asm
.for <var>, <start>, <end> [, <step>]
    ; Loop body
    ; <var> goes from <start> to <end>-1 (exclusive end)
.endfor
```

- `<start>`, `<end>`, `<step>` are evaluated once at loop start
- `<step>` defaults to 1 if not provided
- Loop continues while:
  - If `step > 0`: `var < end`
  - If `step < 0`: `var > end`
- `<step>` of 0 is an error

### `.while` Loop

```asm
.while <condition> [, <var>]
    ; Loop body
    ; Executes while <condition> is true (non-zero)
    ; If supplied, update <var> each iteration starting from 0
.endwhile
```

- `<condition>` is evaluated before each iteration
- If `<var>` is provided, it starts at 0 and increments each iteration
- Be careful of infinite loops!
    - The preprocessor handles this by enforcing a maximum iteration limit (e.g., 1,000,000 iterations)

### `.continue` and `.break`

- `.continue`: Skip remaining code in current iteration, proceed to next
- `.break`: Exit the innermost loop entirely
- Error if used outside a loop

## Implementation Steps

### Step 1: Add Loop State Structures

Add the `loop_type` enum and `loop_state` structure to `preprocessor.hpp`.
Add a loop stack member to the preprocessor class.

### Step 2: Add Loop Helper Methods

- `collect_loop_body()`: Collect tokens until matching end directive
- `process_loop_body()`: Process collected tokens for one iteration
- `is_in_loop()`: Check if currently inside a loop

### Step 3: Implement Loop Handlers

- `handle_repeat_directive()`
- `handle_endrepeat_directive()`
- `handle_for_directive()`
- `handle_endfor_directive()`
- `handle_while_directive()`
- `handle_endwhile_directive()`
- `handle_continue_directive()`
- `handle_break_directive()`

### Step 4: Update Directive Dispatch

Add cases for loop directives in `handle_directive()`.

## Error Conditions

The implementation should detect and report:

1. **Negative repeat count**: `.repeat` with count < 0
2. **Zero step in for loop**: `.for` with step = 0
3. **Unmatched end directives**: `.endrepeat`/`.endfor`/`.endwhile` without start
4. **Unclosed loops**: Loop without matching end at EOF
5. **`.continue`/`.break` outside loop**: Control flow outside any loop
6. **Infinite loop detection**: Optional, can use iteration limit

## Examples

### Basic Repeat

```asm
.repeat 3
    nop
.endrepeat
; Output: nop nop nop
```

### Repeat with Variable

```asm
.repeat 4, i
    .byte {i}
.endrepeat
; Output: .byte 0 .byte 1 .byte 2 .byte 3
```

### For Loop

```asm
.for i, 0, 5
    ld r{i}, {i * 10}
.endfor
; Output: ld r0, 0  ld r1, 10  ld r2, 20  ld r3, 30  ld r4, 40
```

### For Loop with Step

```asm
.for i, 10, 0, -2
    .byte {i}
.endfor
; Output: .byte 10 .byte 8 .byte 6 .byte 4 .byte 2
```

### While Loop

```asm
.define COUNT 0
.while COUNT < 3, i
    ld r{i}, {COUNT}
    .define COUNT {COUNT + 1}
.endwhile
; Output: ld r0, 0  ld r1, 1  ld r2, 2
```

### Nested Loops

```asm
.for i, 0, 2
    .for j, 0, 3
        .byte {i * 3 + j}
    .endfor
.endfor
; Output: .byte 0 .byte 1 .byte 2 .byte 3 .byte 4 .byte 5
```

### Continue and Break

```asm
.for i, 0, 10
    .if i == 3
        .continue
    .endif
    .if i == 7
        .break
    .endif
    .byte {i}
.endfor
; Output: .byte 0 .byte 1 .byte 2 .byte 4 .byte 5 .byte 6
```

## Testing

Test cases should verify:

1. Basic `.repeat` with count
2. `.repeat` with loop variable
3. `.repeat` with count 0 (skip body)
4. Basic `.for` loop
5. `.for` with negative step
6. `.for` with step that skips end
7. Basic `.while` loop
8. `.while` with loop variable
9. Nested loops (same type and mixed)
10. `.continue` directive
11. `.break` directive
12. `.continue`/`.break` in nested loops
13. Error cases
