# Step 6: Conditional Assembly

This document provides detailed step-by-step instructions for implementing
conditional assembly in the G10 Assembler's preprocessor.

## Overview

Conditional assembly allows sections of source code to be included or excluded
based on compile-time conditions. The G10 preprocessor supports the following
conditional directives:

| Directive | Description |
|-----------|-------------|
| `.if <condition>` | Begin conditional block if condition is true (non-zero) |
| `.ifdef <NAME>` | Begin conditional block if macro NAME is defined |
| `.ifndef <NAME>` | Begin conditional block if macro NAME is NOT defined |
| `.elif <condition>` / `.elseif <condition>` | Alternative condition if previous was false |
| `.else` | Execute if all previous conditions were false |
| `.endif` / `.endc` | End conditional block |

## References

- [G10 Preprocessor Language Specification](../docs/g10pp.specification.md)
- [G10 Code Style Guide](../docs/g10.code_style.md)
- [Preprocessor Header](../projects/g10asm/preprocessor.hpp)
- [Preprocessor Implementation](../projects/g10asm/preprocessor.cpp)

## Implementation Design

### Conditional State Stack

Since conditionals can be nested, we need a stack to track the state of each
conditional block. Each entry on the stack contains:

```cpp
struct conditional_state
{
    bool condition_met;     // Has any branch in this block been taken?
    bool currently_active;  // Is the current branch being processed?
    bool else_seen;         // Have we seen an .else directive?
};
```

### State Transitions

The conditional state machine works as follows:

1. **`.if`, `.ifdef`, `.ifndef`**: Push a new state onto the stack
   - If parent is inactive: new state is inactive regardless of condition
   - If condition is true: set `condition_met = true`, `currently_active = true`
   - If condition is false: set `condition_met = false`, `currently_active = false`

2. **`.elif` / `.elseif`**: Modify top of stack
   - Error if no matching `.if`
   - Error if `.else` was already seen
   - If `condition_met` is true: stay inactive (a branch was already taken)
   - If condition is true: set `condition_met = true`, `currently_active = true`

3. **`.else`**: Modify top of stack
   - Error if no matching `.if`
   - Error if `.else` was already seen
   - Set `else_seen = true`
   - If `condition_met` is false: set `currently_active = true`

4. **`.endif` / `.endc`**: Pop state from stack
   - Error if stack is empty

### Skipping Inactive Sections

When `currently_active` is false, the preprocessor must skip tokens until:
- A matching `.elif`, `.elseif`, `.else`, or `.endif`/`.endc` is found
- Nested conditionals must be tracked to ensure proper matching

## Implementation Steps

### Step 1: Add Conditional State Structure

Add the `conditional_state` structure and a stack to track nested conditionals
in `preprocessor.hpp`.

### Step 2: Add Helper Methods

Add methods to:
- Check if currently active: `is_conditionally_active()`
- Skip to next conditional directive: `skip_to_next_conditional()`
- Evaluate a condition expression

### Step 3: Implement Directive Handlers

Implement handlers for each conditional directive:
- `handle_if_directive()`
- `handle_ifdef_directive()`
- `handle_ifndef_directive()`
- `handle_elif_directive()`
- `handle_else_directive()`
- `handle_endif_directive()`

### Step 4: Modify Main Loop

Modify the `preprocess()` main loop to check conditional state before
processing each token.

## Behavior Examples

### Basic `.if` / `.endif`

```asm
.define DEBUG 1

.if DEBUG
    ; This code is included
    nop
.endif
```

### `.if` / `.else` / `.endif`

```asm
.define MODE 0

.if MODE
    ; Mode is non-zero
    ld r0, 1
.else
    ; Mode is zero
    ld r0, 0
.endif
```

### `.ifdef` / `.ifndef`

```asm
.define FEATURE_ENABLED 1

.ifdef FEATURE_ENABLED
    ; Feature is enabled
    call feature_init
.endif

.ifndef DEBUG
    ; DEBUG is not defined
    ; Release mode code
.endif
```

### Chained `.elif`

```asm
.define VERSION 2

.if VERSION == 1
    ld r0, 1
.elif VERSION == 2
    ld r0, 2
.elif VERSION == 3
    ld r0, 3
.else
    ld r0, 0
.endif
```

### Nested Conditionals

```asm
.define OUTER 1
.define INNER 1

.if OUTER
    ; Outer is true
    .if INNER
        ; Both outer and inner are true
        nop
    .endif
.endif
```

## Error Conditions

The implementation should detect and report:

1. **Unmatched `.endif`**: `.endif` without corresponding `.if`
2. **Unmatched `.else`**: `.else` without corresponding `.if`
3. **Duplicate `.else`**: More than one `.else` in a conditional block
4. **`.elif` after `.else`**: `.elif` appearing after `.else`
5. **Unclosed conditional**: `.if` without matching `.endif` at end of file
6. **`.elif`/`.else` without `.if`**: Appearing outside any conditional block

## Testing

Test cases should verify:

1. Basic `.if` with true condition
2. Basic `.if` with false condition
3. `.if` / `.else` branching
4. `.ifdef` for defined macros
5. `.ifdef` for undefined macros
6. `.ifndef` for defined macros
7. `.ifndef` for undefined macros
8. Chained `.elif` with various conditions
9. Nested conditionals (multiple levels)
10. Expressions in conditions (comparisons, logical operators)
11. Error cases (unmatched directives)
