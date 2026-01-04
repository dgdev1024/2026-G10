# Step 2: Text-Substitution Macros (`.define` and `.undef`/`.purge`)

This document provides detailed step-by-step instructions for implementing
text-substitution macros in the G10 Assembler's preprocessor. Text-substitution
macros are the most fundamental macro type, allowing simple token replacement
throughout the source code.

## Overview

Text-substitution macros are defined using the `.define` directive and perform
simple text replacement. When the preprocessor encounters a defined macro name
in the source code, it replaces it with the defined replacement text.

According to the G10 Preprocessor Specification:

> `.define DEF_NAME REPLACEMENT_TEXT`: Defines a text-substitution macro.
> Whenever the preprocessor encounters `DEF_NAME` in the source code, it
> replaces it with `REPLACEMENT_TEXT`. The `REPLACEMENT_TEXT` can be any
> sequence of text, starting from the first non-whitespace character after
> `DEF_NAME` to the end of the line or file.

Macros can be undefined using `.undef` or `.purge` directives.

## References

- [G10 Preprocessor Language Specification](../docs/g10pp.specification.md)
- [G10 Code Style Guide](../docs/g10.code_style.md)
- [Preprocessor Header](../projects/g10asm/preprocessor.hpp)
- [Preprocessor Implementation](../projects/g10asm/preprocessor.cpp)
- [Preprocessor Macros Header](../projects/g10asm/preprocessor_macros.hpp)

## Macro Naming Rules

From the specification, macro names must:

- Begin with a letter (`A-Z`, `a-z`) or underscore (`_`)
- May contain letters, digits (`0-9`), and underscores
- Are case-sensitive (`MyMacro` and `MYMACRO` are different)
- Cannot be a reserved keyword or instruction mnemonic
- Cannot begin with double underscore (`__`) - reserved for built-in macros

## Implementation Details

### 1. Text-Substitution Macro Structure

Create a structure to store text-substitution macro information:

```cpp
struct text_sub_macro final
{
    std::string name {};                // The macro's name
    std::vector<token> replacement {};  // Replacement tokens
    std::string source_file {};         // File where macro was defined
    std::size_t source_line { 0 };      // Line where macro was defined
};
```

### 2. Macro Table

The `pp_macro_table` class should manage text-substitution macros:

- Store macros in a map for O(1) lookup by name
- Provide methods to define, lookup, and undefine macros
- Validate macro names according to the naming rules

### 3. Directive Detection

Modify the main preprocessing loop to detect preprocessor directives. When
encountering a `keyword` token with `keyword_type::preprocessor_directive`:

1. Check the directive type
2. For `.define`: parse the macro name and replacement text
3. For `.undef`/`.purge`: remove the macro from the table

### 4. Macro Expansion

During token iteration, when encountering an `identifier` token:

1. Check if it matches a defined macro name
2. If so, replace the identifier with the macro's replacement tokens
3. Continue processing from the replacement tokens

### 5. Collecting Replacement Tokens

For `.define`, collect all tokens from after the macro name until:
- A newline is encountered (not preceded by line continuation)
- End of input is reached

The collected tokens become the macro's replacement text.

## Implementation Steps

### Step 2.1: Update preprocessor_macros.hpp

Add the text-substitution macro structure and update the macro table class.

### Step 2.2: Implement Macro Table Methods

Implement methods in preprocessor_macros.cpp:
- `define_text_sub_macro()` - Add a new macro
- `lookup_text_sub_macro()` - Find a macro by name  
- `undefine_macro()` - Remove a macro
- `is_macro_defined()` - Check if a macro exists

### Step 2.3: Add Directive Handling to Preprocessor

Add methods to handle preprocessor directives:
- `handle_directive()` - Main directive dispatcher
- `handle_define_directive()` - Process `.define`
- `handle_undef_directive()` - Process `.undef`/`.purge`

### Step 2.4: Add Macro Expansion

Add method to expand macros when identifiers are encountered:
- `try_expand_macro()` - Check and expand if identifier is a macro

### Step 2.5: Update Main Loop

Modify `preprocess()` to:
1. Check for directives before outputting tokens
2. Check for macro expansion on identifiers

## Error Handling

The following errors should be reported:

1. **Invalid macro name**: Name doesn't follow naming rules
2. **Reserved name**: Name starts with `__` or is a keyword
3. **Undefined macro**: Trying to `.undef` a macro that doesn't exist (warning)
4. **Missing macro name**: `.define` without a name

## Testing

Create test files to verify:

1. Basic macro definition and expansion
2. Macro with multi-token replacement
3. Macro undefined before use
4. Multiple macro definitions
5. Macro expansion in various contexts
6. Error cases (invalid names, etc.)

## Verification Commands

```bash
# Show preprocessed string output
g10asm --preprocess -s <file>

# Show post-preprocessing lexer tokens
g10asm --preprocess --lex -s <file>
```

## Success Criteria

1. `.define` correctly stores text-substitution macros
2. Macro names are expanded wherever they appear as identifiers
3. `.undef`/`.purge` correctly removes macros
4. Proper error messages for invalid operations
5. All test cases produce expected output
6. No regressions in Step 1 tests
