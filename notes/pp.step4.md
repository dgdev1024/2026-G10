# Step 4: Extended Braced Expression Interpolation

This document provides detailed step-by-step instructions for implementing
extended braced expression interpolation in the G10 Assembler's preprocessor.
This step extends the braced expression feature to support interpolation within
identifiers and string literals.

## Overview

Building upon the basic braced expression evaluation from Step 3, this step
enables expressions to be interpolated:

1. **Within Identifiers**: Allows dynamic label/symbol name generation
   - `{21 * 2}_start` → `42_start`
   - `label_{50 / 2}_suffix` → `label_25_suffix`
   - `prefix_{VALUE}` → `prefix_42` (if VALUE is defined as 42)

2. **Within String Literals**: Allows dynamic string content
   - `"The answer is {2 + (10 * 4)}."` → `"The answer is 42."`
   - `"Value: {MY_CONST}"` → `"Value: 100"` (if MY_CONST is 100)

## References

- [G10 Preprocessor Language Specification](../docs/g10pp.specification.md)
- [G10 Code Style Guide](../docs/g10.code_style.md)
- [Preprocessor Header](../projects/g10asm/preprocessor.hpp)
- [Preprocessor Implementation](../projects/g10asm/preprocessor.cpp)
- [Step 3: Braced Expression Interpolation](./pp.step3.md)

## Implementation Details

### Token Pattern Analysis

When the lexer processes source code, it creates tokens. The preprocessor
receives these tokens and must recognize patterns that indicate interpolation:

#### Identifier Interpolation Patterns

1. **Braced expression followed by identifier-like text**:
   - Token sequence: `{`, expr tokens..., `}`, `identifier`
   - Example: `{42}_start` → tokens: `{`, `42`, `}`, `_start`

2. **Identifier followed by braced expression**:
   - Token sequence: `identifier`, `{`, expr tokens..., `}`
   - Example: `label_{42}` → tokens: `label_`, `{`, `42`, `}`

3. **Identifier with embedded braced expression**:
   - Token sequence: `identifier`, `{`, expr tokens..., `}`, `identifier`
   - Example: `a_{42}_b` → tokens: `a_`, `{`, `42`, `}`, `_b`

#### String Interpolation

String literals are single tokens with the content stored in `lexeme`.
The preprocessor must scan the string content for `{...}` patterns and
evaluate them inline.

### Implementation Steps

#### Step 4.1: Token Concatenation Detection

Add logic to detect when consecutive tokens should be concatenated:
- An identifier immediately adjacent to a braced expression (no space)
- The result forms a new identifier

#### Step 4.2: String Interpolation

Process string literal tokens to find and evaluate embedded expressions:
1. Scan the string content for `{` characters
2. Extract the expression content until matching `}`
3. Tokenize and evaluate the expression
4. Replace `{expr}` with the result
5. Handle nested braces and escape sequences

### Adjacency Detection

To detect if tokens are adjacent (no whitespace between them), we need to
check if the ending position of one token is immediately followed by the
starting position of the next token. This is determined by:

```
token1.source_column + token1.lexeme.length() == token2.source_column
AND token1.source_line == token2.source_line
```

## Error Handling

1. **Unmatched braces in strings**: Report error with location
2. **Invalid expression in string**: Report evaluation error
3. **Empty interpolation**: Handle `{}` in strings gracefully

## Testing

Test cases should verify:
1. Braced expression at start of identifier
2. Braced expression at end of identifier
3. Braced expression in middle of identifier
4. Multiple interpolations in one identifier
5. Basic string interpolation
6. Multiple interpolations in one string
7. Macro references in string interpolation
8. Escaped braces in strings (future consideration)

## Success Criteria

1. `{expr}_suffix` produces correct identifier
2. `prefix_{expr}` produces correct identifier
3. `prefix_{expr}_suffix` produces correct identifier
4. `"text {expr} more"` produces correct string
5. All previous tests still pass
6. Error messages for malformed interpolations
