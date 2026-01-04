# Step 3: Braced Expression Interpolation

This document provides detailed step-by-step instructions for implementing
braced expression interpolation in the G10 Assembler's preprocessor. Expressions
enclosed in curly braces `{}` will be evaluated by the preprocessor and replaced
with their resulting values.

## Overview

Braced expression interpolation allows the preprocessor to evaluate expressions
at preprocessing time and emit the result directly into the output. This is a
fundamental feature that enables computed values in macros and throughout the
source code.

Examples:
- `{3 + 2}` evaluates to `5`
- `{0xFF & 0x0F}` evaluates to `15`
- `.define VALUE {1 + 2 * 3}` - VALUE expands to `7` (not `1 + 2 * 3`)

## References

- [G10 Preprocessor Language Specification](../docs/g10pp.specification.md)
- [G10 Code Style Guide](../docs/g10.code_style.md)
- [Preprocessor Header](../projects/g10asm/preprocessor.hpp)
- [Preprocessor Implementation](../projects/g10asm/preprocessor.cpp)
- [Preprocessor Evaluator Header](../projects/g10asm/preprocessor_evaluator.hpp)
- [Preprocessor Values Header](../projects/g10asm/preprocessor_values.hpp)

## Expression Syntax

The preprocessor supports the following in expressions:

### Literals
- Integer literals: `42`, `0xFF`, `0b1010`, `0o755`
- Character literals: `'A'` (evaluates to ASCII value 65)
- Number literals: `3.14` (fixed-point representation)

### Operators (in order of precedence, highest to lowest)
1. Parentheses: `()`
2. Unary: `+`, `-`, `~`, `!`
3. Exponentiation: `**`
4. Multiplicative: `*`, `/`, `%`
5. Additive: `+`, `-`
6. Shift: `<<`, `>>`
7. Comparison: `<`, `<=`, `>`, `>=`
8. Equality: `==`, `!=`
9. Bitwise AND: `&`
10. Bitwise XOR: `^`
11. Bitwise OR: `|`
12. Logical AND: `&&`
13. Logical OR: `||`

## Implementation Details

### 1. Expression Evaluator

The `pp_evaluator` class needs to be fully implemented to:
- Parse a sequence of tokens representing an expression
- Evaluate the expression using operator precedence
- Return the result as a `pp_value`

### 2. Braced Expression Detection

The preprocessor needs to detect left brace `{` tokens and:
1. Collect all tokens until the matching right brace `}`
2. Handle nested braces correctly
3. Pass the expression tokens to the evaluator
4. Emit the result string to the output

### 3. Value Stringification

The evaluated result needs to be converted to a string:
- Integers: Output as decimal (e.g., `7`, `-42`)
- Numbers: Output as decimal with appropriate precision
- Booleans: Output as `1` (true) or `0` (false)
- Strings: Output the string content

## Implementation Steps

### Step 3.1: Implement the Expression Evaluator

Update `pp_evaluator` to parse and evaluate expressions using recursive descent
parsing with proper operator precedence.

### Step 3.2: Add Braced Expression Handling

Add a method `handle_braced_expression()` to the preprocessor that:
1. Detects `{` token
2. Collects tokens until matching `}`
3. Evaluates the expression
4. Outputs the result

### Step 3.3: Value to String Conversion

Add a helper function to convert `pp_value` to a string representation.

## Error Handling

The following errors should be reported:
1. **Unmatched brace**: Opening `{` without closing `}`
2. **Invalid expression**: Syntax error in expression
3. **Division by zero**: Attempted division or modulo by zero
4. **Type error**: Invalid operation for value types

## Testing

Create test files to verify:
1. Basic arithmetic expressions
2. Operator precedence
3. Bitwise operations
4. Comparison and logical operations
5. Expressions in macro definitions
6. Nested parentheses
7. Error cases

## Success Criteria

1. `{expr}` is evaluated and replaced with the result
2. Expressions in `.define` are evaluated at definition time
3. Proper operator precedence is followed
4. Error messages for invalid expressions
5. All test cases produce expected output
6. No regressions in previous step tests
