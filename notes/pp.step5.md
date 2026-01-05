# Step 5: Built-in Functions

This document provides detailed step-by-step instructions for implementing
built-in functions in the G10 Assembler's preprocessor expression evaluator.

## Overview

The G10 preprocessor provides a rich set of built-in functions for use within
braced expressions. These functions are organized into the following categories:

1. **Integer Functions**: `high`, `low`, `bitwidth`, `abs`, `min`, `max`, `clamp`
2. **Fixed-Point Arithmetic**: `fmul`, `fdiv`, `fmod`
3. **Fixed-Point Conversion**: `fint`, `ffrac`, `round`, `ceil`, `floor`, `trunc`
4. **Fixed-Point Math**: `pow`, `log`, `sqrt`, `exp`, `ln`, `log2`, `log10`
5. **Trigonometric**: `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `atan2`
6. **String Functions**: `strlen`, `strcmp`, `substr`, `indexof`, `toupper`,
   `tolower`, `concat`
7. **Miscellaneous**: `defined`, `typeof`

## References

- [G10 Preprocessor Language Specification](../docs/g10pp.specification.md)
- [G10 Code Style Guide](../docs/g10.code_style.md)
- [Preprocessor Evaluator Header](../projects/g10asm/preprocessor_evaluator.hpp)
- [Preprocessor Evaluator Implementation](../projects/g10asm/preprocessor_evaluator.cpp)
- [Step 3: Braced Expression Interpolation](./pp.step3.md)

## Fixed-Point Format

The G10 preprocessor uses **32.32 fixed-point format**:
- Upper 32 bits: signed integer part
- Lower 32 bits: fractional part
- The value `1.0` is represented as `0x100000000` (2^32)

Key constants:
```cpp
constexpr std::int64_t FP_ONE = 1LL << 32;          // 1.0 in fixed-point
constexpr std::int64_t FP_HALF = 1LL << 31;         // 0.5 in fixed-point
constexpr std::int64_t FP_FRAC_MASK = 0xFFFFFFFF;   // Mask for fractional part
```

## Implementation Details

### Function Call Parsing

When the evaluator encounters an identifier followed by `(`, it should recognize
this as a function call. The implementation involves:

1. Parse the function name (identifier)
2. Consume the opening parenthesis `(`
3. Parse arguments as comma-separated expressions
4. Consume the closing parenthesis `)`
5. Look up and execute the function with the parsed arguments

### Function Dispatch

Create a dispatch mechanism that maps function names to their implementations.
Each function should:

1. Validate the number of arguments
2. Validate argument types where applicable
3. Perform the computation
4. Return the result as a `pp_value`

### Trigonometric Functions

Trigonometric functions use **turns** as the angle unit (1.0 turn = 360° = 2π radians).
Implementation approaches:

1. **Table-based**: Pre-compute sine values and use interpolation
2. **CORDIC algorithm**: Iterative rotation for accurate fixed-point results
3. **Taylor series**: Polynomial approximation (less suitable for fixed-point)

For this implementation, we'll use a combination of lookup tables and
mathematical identities.

## Function Specifications

### Integer Functions

| Function | Arguments | Description |
|----------|-----------|-------------|
| `high(n)` | 1 integer | Upper 8 bits: `(n >> 8) & 0xFF` |
| `low(n)` | 1 integer | Lower 8 bits: `n & 0xFF` |
| `bitwidth(n)` | 1 integer | Number of bits to represent n |
| `abs(n)` | 1 integer | Absolute value |
| `min(a, b)` | 2 integers | Smaller of a and b |
| `max(a, b)` | 2 integers | Larger of a and b |
| `clamp(v, lo, hi)` | 3 integers | v clamped to [lo, hi] |

### Fixed-Point Functions

| Function | Arguments | Description |
|----------|-----------|-------------|
| `fmul(x, y)` | 2 fixed-point | Fixed-point multiplication |
| `fdiv(x, y)` | 2 fixed-point | Fixed-point division |
| `fmod(x, y)` | 2 fixed-point | Fixed-point modulo |
| `fint(fp)` | 1 fixed-point | Extract integer part |
| `ffrac(fp)` | 1 fixed-point | Extract fractional part |
| `round(fp)` | 1 fixed-point | Round to nearest integer |
| `ceil(fp)` | 1 fixed-point | Round up |
| `floor(fp)` | 1 fixed-point | Round down |
| `trunc(fp)` | 1 fixed-point | Truncate toward zero |

### Math Functions

| Function | Arguments | Description |
|----------|-----------|-------------|
| `sqrt(x)` | 1 fixed-point | Square root |
| `pow(x, y)` | 2 fixed-point | x raised to power y |
| `exp(x)` | 1 fixed-point | e^x |
| `ln(x)` | 1 fixed-point | Natural logarithm |
| `log2(x)` | 1 fixed-point | Base-2 logarithm |
| `log10(x)` | 1 fixed-point | Base-10 logarithm |
| `log(x, base)` | 2 fixed-point | Logarithm with base |

### Trigonometric Functions

| Function | Arguments | Description |
|----------|-----------|-------------|
| `sin(angle)` | 1 fixed-point (turns) | Sine |
| `cos(angle)` | 1 fixed-point (turns) | Cosine |
| `tan(angle)` | 1 fixed-point (turns) | Tangent |
| `asin(x)` | 1 fixed-point | Arc sine (returns turns) |
| `acos(x)` | 1 fixed-point | Arc cosine (returns turns) |
| `atan(x)` | 1 fixed-point | Arc tangent (returns turns) |
| `atan2(y, x)` | 2 fixed-point | Two-argument arc tangent |

### String Functions

| Function | Arguments | Description |
|----------|-----------|-------------|
| `strlen(str)` | 1 string | String length |
| `strcmp(s1, s2)` | 2 strings | Compare strings |
| `substr(str, start, [len])` | 2-3 | Substring extraction |
| `indexof(str, search)` | 2 strings | Find substring |
| `toupper(str)` | 1 string | Convert to uppercase |
| `tolower(str)` | 1 string | Convert to lowercase |
| `concat(s1, s2, ...)` | 2+ strings | Concatenate strings |

### Miscellaneous Functions

| Function | Arguments | Description |
|----------|-----------|-------------|
| `defined(name)` | 1 identifier | Check if macro is defined |
| `typeof(expr)` | 1 expression | Get type as string |

## Error Handling

Functions should report clear errors for:
- Wrong number of arguments
- Invalid argument types
- Domain errors (e.g., sqrt of negative, log of non-positive)
- Overflow conditions

## Testing

Test cases should verify:
1. Each function with valid inputs
2. Edge cases (zero, negative, overflow)
3. Error conditions
4. Combination with other expression features
5. Functions in string interpolation (no quotes on string results)
6. Functions in identifier interpolation (no quotes on string results)

### Context-Dependent String Quoting

When string-returning functions are used in different contexts, the quoting
behavior differs:

| Context | Example | Result |
|---------|---------|--------|
| Standalone braced expression | `.string {toupper("hi")}` | `.string "HI"` |
| String interpolation | `.string "Say: {toupper("hi")}"` | `.string "Say: HI"` |
| Identifier interpolation | `{toupper("hi")}_label:` | `HI_label:` |

This ensures that string function results integrate naturally into their
surrounding context without unwanted quote characters.
