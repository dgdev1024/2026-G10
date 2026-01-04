# Step 1: Basic Token-to-String Output and Line Continuation Handling

This document provides detailed step-by-step instructions for implementing the
foundational preprocessing functionality in the G10 Assembler Tool (`g10asm`).
This step establishes the core preprocessing loop that iterates through tokens
and builds the output string, as well as handling line continuation with the
backslash (`\`) character.

## Overview

The G10 preprocessor takes tokens from the initial lexical analysis stage and
produces a preprocessed string output. Before implementing any complex features
like macros or conditionals, we need to establish:

1. **Basic Token Iteration**: A loop that walks through input tokens
2. **Output String Building**: Converting tokens back to text for the output
3. **Line Continuation Handling**: Processing the backslash (`\`) character at
   end of lines to join multiple physical lines into a single logical line
4. **Newline Preservation**: Preserving newlines in the output for proper line
   tracking

## References

- [G10 Preprocessor Language Specification](../docs/g10pp.specification.md)
- [G10 Code Style Guide](../docs/g10.code_style.md)
- [Preprocessor Header](../projects/g10asm/preprocessor.hpp)
- [Preprocessor Implementation](../projects/g10asm/preprocessor.cpp)
- [Token Definitions](../projects/g10asm/token.hpp)

## Implementation Details

### 1. Token Filtering Adjustment

The current `filter_input_tokens()` method removes `new_line` and `end_of_file`
tokens. However, we need to preserve `new_line` tokens to:
- Properly handle line continuation
- Preserve line structure in the output
- Support diagnostics with accurate line numbers

We should keep the `end_of_file` token removal but preserve `new_line` tokens.

### 2. Core Preprocessing Loop

The `preprocess()` method should implement a main loop that:
1. Tracks the current position in the token list
2. Processes each token based on its type
3. Handles special cases (line continuation, directives, etc.)
4. Builds the output string

### 3. Line Continuation Handling

According to the specification (Section "Line Continuation"):

> Long lines can be split across multiple physical lines using the backslash
> (`\`) character. When a backslash appears at the end of a line (immediately
> before the newline character), the backslash and newline are removed, and
> the next line is appended to the current line.

The implementation should:
1. Detect a `backslash` token followed by a `new_line` token
2. Skip both tokens (removing them from output)
3. Continue processing the next line
4. Report an error if whitespace appears between backslash and newline
   (this is handled by the lexer, but we verify the pattern)

### 4. Token-to-String Conversion

For each token that should appear in the output, we need to convert it back
to its string representation. The token's `lexeme` field contains the original
source text, which should be used for most tokens.

Special handling is needed for:
- **Newlines**: Output actual newline characters to preserve line structure
- **String literals**: Use the lexeme directly (includes quotes)
- **Character literals**: Use the lexeme directly (includes quotes)
- **Spacing**: Add appropriate spacing between tokens

### 5. Spacing Between Tokens

To produce readable output and avoid token concatenation issues, we need smart
spacing:
- Add a space between most tokens
- No space after opening brackets/parentheses
- No space before closing brackets/parentheses
- No space before commas or colons
- Newlines reset spacing state

## Code Implementation

### Step 1.1: Add Helper Members to Preprocessor

Add the following private members to the `preprocessor` class in
`preprocessor.hpp`:

```cpp
/**
 * @brief   The current index position in the input token list.
 */
std::size_t m_current_index { 0 };

/**
 * @brief   Indicates whether a space should be added before the next token
 *          in the output.
 */
bool m_needs_space { false };
```

### Step 1.2: Add Helper Methods

Add these private method declarations to `preprocessor.hpp`:

```cpp
/**
 * @brief   Returns the current token at the current index position.
 *
 * @return  A constant reference to the current token, or an error if out
 *          of bounds.
 */
auto current_token () const -> g10::result_cref<token>;

/**
 * @brief   Peeks at a token at the specified offset from the current position.
 *
 * @param   offset  The offset from the current position.
 *
 * @return  A constant reference to the token at the offset, or an error if
 *          out of bounds.
 */
auto peek_token (std::int64_t offset = 0) const -> g10::result_cref<token>;

/**
 * @brief   Advances the current index position by the specified count.
 *
 * @param   count   The number of positions to advance.
 */
auto advance (std::size_t count = 1) -> void;

/**
 * @brief   Checks if the current position is at the end of the token list.
 *
 * @return  True if at end, false otherwise.
 */
auto is_at_end () const -> bool;

/**
 * @brief   Appends a token's lexeme to the output string with appropriate
 *          spacing.
 *
 * @param   tok     The token to append.
 */
auto append_token (const token& tok) -> void;

/**
 * @brief   Appends a string directly to the output.
 *
 * @param   str     The string to append.
 */
auto append_string (std::string_view str) -> void;

/**
 * @brief   Appends a newline to the output and resets spacing state.
 */
auto append_newline () -> void;

/**
 * @brief   Handles line continuation (backslash followed by newline).
 *
 * @return  True if line continuation was handled, false otherwise.
 */
auto handle_line_continuation () -> bool;

/**
 * @brief   Determines if a space should be added before the given token type.
 *
 * @param   type    The token type to check.
 *
 * @return  True if no space should precede this token type.
 */
auto is_no_space_before (token_type type) const -> bool;

/**
 * @brief   Determines if a space should be added after the given token type.
 *
 * @param   type    The token type to check.
 *
 * @return  True if no space should follow this token type.
 */
auto is_no_space_after (token_type type) const -> bool;
```

### Step 1.3: Update filter_input_tokens()

Modify the filter to keep `new_line` tokens:

```cpp
auto preprocessor::filter_input_tokens (const std::vector<token>& tokens) 
    -> g10::result<void>
{
    m_input_tokens.clear();

    for (const auto& tok : tokens)
    {
        switch (tok.type)
        {
            case token_type::end_of_file:
                // Skip end-of-file tokens
                break;

            default:
                m_input_tokens.push_back(tok);
                break;
        }
    }

    return {};
}
```

### Step 1.4: Implement Helper Methods

Implement the helper methods in `preprocessor.cpp`:

```cpp
auto preprocessor::current_token () const -> g10::result_cref<token>
{
    if (m_current_index >= m_input_tokens.size())
    {
        return g10::error("Current token index out of bounds.");
    }
    return std::cref(m_input_tokens[m_current_index]);
}

auto preprocessor::peek_token (std::int64_t offset) const 
    -> g10::result_cref<token>
{
    std::int64_t target = static_cast<std::int64_t>(m_current_index) + offset;
    if (target < 0 || static_cast<std::size_t>(target) >= m_input_tokens.size())
    {
        return g10::error("Token peek offset out of bounds.");
    }
    return std::cref(m_input_tokens[static_cast<std::size_t>(target)]);
}

auto preprocessor::advance (std::size_t count) -> void
{
    m_current_index += count;
    if (m_current_index > m_input_tokens.size())
    {
        m_current_index = m_input_tokens.size();
    }
}

auto preprocessor::is_at_end () const -> bool
{
    return m_current_index >= m_input_tokens.size();
}

auto preprocessor::is_no_space_before (token_type type) const -> bool
{
    switch (type)
    {
        case token_type::comma:
        case token_type::colon:
        case token_type::right_parenthesis:
        case token_type::right_bracket:
        case token_type::right_brace:
        case token_type::new_line:
            return true;
        default:
            return false;
    }
}

auto preprocessor::is_no_space_after (token_type type) const -> bool
{
    switch (type)
    {
        case token_type::left_parenthesis:
        case token_type::left_bracket:
        case token_type::left_brace:
        case token_type::new_line:
            return true;
        default:
            return false;
    }
}

auto preprocessor::append_token (const token& tok) -> void
{
    // Add space if needed and token type allows
    if (m_needs_space && !is_no_space_before(tok.type))
    {
        m_output_string += ' ';
    }

    // Append the token's lexeme
    m_output_string += tok.lexeme;

    // Update spacing state
    m_needs_space = !is_no_space_after(tok.type);
}

auto preprocessor::append_string (std::string_view str) -> void
{
    m_output_string += str;
}

auto preprocessor::append_newline () -> void
{
    m_output_string += '\n';
    m_needs_space = false;
}

auto preprocessor::handle_line_continuation () -> bool
{
    auto tok_result = current_token();
    if (!tok_result.has_value())
    {
        return false;
    }

    const token& tok = tok_result.value();

    // Check for backslash token
    if (tok.type != token_type::backslash)
    {
        return false;
    }

    // Check if next token is a newline
    auto next_result = peek_token(1);
    if (!next_result.has_value())
    {
        return false;
    }

    const token& next_tok = next_result.value();
    if (next_tok.type != token_type::new_line)
    {
        return false;
    }

    // Line continuation found - skip both backslash and newline
    advance(2);
    return true;
}
```

### Step 1.5: Implement Main Preprocessing Loop

Update the `preprocess()` method:

```cpp
auto preprocessor::preprocess () -> g10::result<void>
{
    m_current_index = 0;
    m_output_string.clear();
    m_needs_space = false;

    while (!is_at_end())
    {
        // Handle line continuation first
        if (handle_line_continuation())
        {
            continue;
        }

        auto tok_result = current_token();
        if (!tok_result.has_value())
        {
            break;
        }

        const token& tok = tok_result.value();

        // Handle newlines specially
        if (tok.type == token_type::new_line)
        {
            append_newline();
            advance();
            continue;
        }

        // For all other tokens, append to output
        append_token(tok);
        advance();
    }

    m_good = true;
    return {};
}
```

## Testing

Create test files in `notes/pp.tests/step1/` to verify the implementation.

### Test 1: Basic Token Output (`test1_basic.asm`)

Test that basic tokens are correctly converted back to string output.

### Test 2: Line Continuation (`test2_continuation.asm`)

Test that backslash line continuation works correctly.

### Test 3: Multiple Line Continuations (`test3_multi_continuation.asm`)

Test multiple consecutive line continuations.

### Test 4: Mixed Content (`test4_mixed.asm`)

Test a combination of regular code, comments, and line continuations.

## Verification Commands

Run the following commands to verify the implementation:

```bash
# Build the project
./scripts/build.sh

# Test basic output (shows preprocessed string)
./build/bin/linux-debug/g10asm --preprocess -s notes/pp.tests/step1/test1_basic.asm

# Test with lexer output (shows tokens after preprocessing)
./build/bin/linux-debug/g10asm --preprocess --lex -s notes/pp.tests/step1/test1_basic.asm
```

## Success Criteria

1. Tokens are correctly converted back to string output
2. Line continuations (backslash + newline) are properly removed
3. Spacing between tokens is appropriate
4. Newlines are preserved in the output
5. No errors during preprocessing of valid input
6. All test cases produce expected output
