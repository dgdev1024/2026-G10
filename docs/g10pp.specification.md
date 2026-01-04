# Specification: G10 Assembly Language Preprocessor

Below is a specification of `g10pp`, the preprocessing language used by the
G10 CPU assembler tool (`g10asm`). This document outlines the syntax, semantics,
and features of the G10 assembly preprocessor.

Although `g10pp` is intended for use with the G10 assembly language and assembler
tool, it is designed to be a general-purpose preprocessing language that could
be adapted for use with other programming languages or assembly languages in
the future.

## Overview

In the G10 assembly process, the preprocessor is second stage in the assembly
pipeline, coming after the initial lexical analysis stage, and before the
secondary lexical analysis and parsing stages.

The preprocessor processes the tokens produced during the initial lexical
analysis stage, handling preprocessor directives, macro definitions and
expansions, conditional assembly, loop constructs, file inclusions and all
relevant expression evaluations.

When finished, the preprocessor outputs a string of preprocessed assembly
source code, which is then fed into the secondary lexical analysis to produce
tokens to be fed into the parser for further processing.

## Preprocessor Directives

The G10 assembly preprocessor supports a variety of directives, each starting
with a dot (`.`). Many (but not all) directives are handled by the preprocessor
before any other stage of assembly. Below is a list of the directives handled
by the preprocessor, along with their syntax and semantics:

- `.pragma <name> [args...]`: Sets various preprocessor options or behaviors.
    The `<name>` specifies the pragma to set, and optional `args` can be
    provided depending on the pragma. See the **Pragmas** section later in
    this document for more details.
- `.include <filename>`: Includes the contents of the specified file at
    this location in the source code. The `<filename>` can be a string literal
    (enclosed in double quotes) or an expression that evaluates to a string.
- `.define DEF_NAME REPLACEMENT_TEXT`: Defines a text-substitution macro. Whenever
    the preprocessor encounters `DEF_NAME` in the source code, it replaces it
    with `REPLACEMENT_TEXT`. The `REPLACEMENT_TEXT` can be any sequence of text,
    starting from the first non-whitespace character after `DEF_NAME` to the end
    of the line or file.
- `.macro MACRO_NAME [PARAM1, PARAM2, ...]`: Begins the definition of a
    parameterized macro named `MACRO_NAME`, followed by an optional, comma-separated
    list of named parameters. The body of the macro follows on subsequent lines,
    until the corresponding `.endm` or `.endmacro` directive is encountered. Within
    the macro body, parameters can be referenced using their names or by their
    positional index (starting from `1`). The **Macros** section later in this
    document provides more details on parameterized macros.
    - **Note**: Macros cannot be nested; that is, you cannot define a macro
        (text-sub or parameterized) within the body of another parameterized macro.
- `.shift <count>`: Shifts the list of all arguments passed to the current
    parameterized macro to the left by `<count>` positions. This allows for
    variadic macros, where a macro can accept a variable number of arguments.
    After shifting, the first `<count>` arguments are discarded, and the
    remaining arguments are shifted left. The special parameter `@argc` is
    updated to reflect the new number of arguments. `<count>` must be a
    non-negative integer literal or an expression that evaluates to a
    non-negative integer. This directive can only be used within the body of
    a parameterized macro.
- `.endm`, `.endmacro`: Ends the definition of a parameterized macro.
- `.undef DEF_NAME`, `.purge DEF_NAME`: Undefines a previously defined
    text-substitution or parameterized macro.
- `.if <condition>`: Begins a conditional assembly block. The `<condition>` is
    an expression that evaluates to a boolean value (non-zero for true, zero
    for false). If the condition is true, the code between this directive and
    the corresponding `.else`, `.elif`/`.elseif`, or `.endc`/`.endif` directive
    is included in the assembly; otherwise, it is skipped.
- `.elif <condition>`, `.elseif <condition>`: Specifies an additional condition
    to check if the previous `.if` or `.elif` condition was false. If this
    condition is true, the code between this directive and the next `.else`,
    `.elif`/`.elseif`, or `.endc`/`.endif` directive is included.
- `.else`: Specifies the code to include if all previous conditions in the
    conditional block were false. The code between this directive and the
    corresponding `.endc`/`.endif` directive is included.
- `.endc`, `.endif`: Ends a conditional assembly block.
- `.rept <count> [, <var>]`, `.repeat <count> [, <var>]`: Begins a repeat loop
    block. The `<count>` is an expression that evaluates to a non-negative
    integer, indicating how many times to repeat the code block. An optional
    loop variable `<var>` can be specified, which will take on values from
    `0` to `<count> - 1` during each iteration of the loop. The code between
    this directive and the corresponding `.endr` or `.endrepeat` directive is
    repeated `<count>` times. If provided, the loop variable `<var>` can be
    referenced within the loop body to obtain the current iteration index.
    - **Note**: The loop variable `<var>` is local to the repeat block and
        will not affect any variables outside of the block.
- `.endr`, `.endrepeat`: Ends the current repeat loop block.
- `.for <var>, <start>, <end> [, <step>]`: Begins a for loop block. The loop
    variable `<var>` is initialized to `<start>`, and the code between this
    directive and the corresponding `.endf` or `.endfor` directive is repeatedly
    included as long as `<var>` is less than `<end>` (`<end>` is exclusive). After each
    iteration, `<var>` is incremented by `<step>`, which defaults to `1` if not
    provided. The loop variable `<var>` can be referenced within the loop body
    to obtain its current value.
    - **Note**: The loop variable `<var>` is local to the for block and will
        not affect any variables outside of the block.
- `.endf`, `.endfor`: Ends the current for loop block.
- `.while <condition> [, <var>]`: Begins a while loop block. The `<condition>` is
    an expression that is evaluated before each iteration of the loop. If the
    condition evaluates to true (non-zero), the code between this directive and
    the corresponding `.endw` or `.endwhile` directive is included; otherwise,
    the loop ends. An optional loop variable `<var>` can be specified, which
    will be incremented by `1` at the end of each iteration. The loop variable
    `<var>` can be referenced within the loop body to obtain its current value.
    - **Note**: The loop variable `<var>` is local to the while block and will
        not affect any variables outside of the block.
- `.endw`, `.endwhile`: Ends the current while loop block.
- `.continue` and `.break`: Control flow directives that can be used within loops
    (`.rept`/`.repeat`, `.for`, `.while`). The `.continue` directive skips the
    remaining code in the current loop iteration and proceeds to the next
    iteration. The `.break` directive exits the loop entirely, skipping any
    remaining iterations.
    - **Note**: The G10 assembler's preprocessor supports nested loops, and the
        `.continue` and `.break` directives will affect the innermost loop
        they are contained within.

### Diagnostic Directives

The following directives allow for user-defined messages, warnings, and errors
during preprocessing:

- `.message <text>`, `.msg <text>`: Outputs an informational message to the
    assembler's standard output. The `<text>` can be a string literal or an
    expression that evaluates to a string. Assembly continues normally after
    the message is displayed.
- `.warning <text>`, `.warn <text>`: Outputs a warning message to the
    assembler's standard error stream. The `<text>` can be a string literal
    or an expression that evaluates to a string. Assembly continues, but the
    warning is recorded and reported in the final assembly summary.
- `.error <text>`, `.err <text>`: Outputs an error message to the assembler's
    standard error stream and terminates assembly. The `<text>` can be a
    string literal or an expression that evaluates to a string.
- `.assert <condition> [, <message>]`: Evaluates the `<condition>` expression.
    If the condition is false (evaluates to zero), assembly is terminated with
    an assertion failure error. An optional `<message>` can be provided to
    give more context about the failed assertion.

Example:

```asm
.define BUFFER_SIZE 256

.assert BUFFER_SIZE >= 64, "Buffer size must be at least 64 bytes"

.if BUFFER_SIZE > 1024
    .warning "Large buffer size may impact performance"
.endif
```

### Unrecognized Preprocessor Directives

It is important to note that not all directives starting with a dot (`.`) are
handled by the preprocessor. Some directives are processed later in the assembly
or compilation pipelines of other tools.

If the preprocessor encounters a directive that it does not recognize, it will
ignore it and leave it unchanged in the source code. This allows for compatibility
with other tools that may have their own directives, such as `g10asm` and its
assembler directives (`.org`, `.byte`, `.word`, etc.), or other tools in the G10 
toolchain. Also note that, in the `g10asm` assembler, dots (`.`) may also begin
user-defined labels, which are also ignored - but may be created or manipulated -
by the preprocessor.

### Include Guards and Pragmas

- `.pragma once`: When placed at the beginning of a file, this directive
    ensures that the file is only included once during the assembly process,
    even if multiple `.include` directives reference it. This is an alternative
    to manual include guards using `.if` and `.define`.
- `.ifdef <NAME>`, `.ifndef <NAME>`: Shorthand conditional directives that
    check whether a macro `<NAME>` is defined (`.ifdef`) or not defined
    (`.ifndef`). These are equivalent to `.if defined(NAME)` and
    `.if !defined(NAME)`, respectively.

Example of manual include guards:

```asm
.ifndef _MY_HEADER_INCLUDED
.define _MY_HEADER_INCLUDED 1

; Header content here...

.endif
```

## Preprocessor Expressions

The G10 assembly preprocessor supports expression evaluation for conditions,
parameters, and inline value computation. Expressions can include integer 
literals, arithmetic operators (`+`, `-`, `*`, `/`, `%`), bitwise operators 
(`&`, `|`, `^`, `~`, `<<`, `>>`), comparison operators (`==`, `!=`, `<`, `>`, 
`<=`, `>=`), logical operators (`&&`, `||`, `!`), and parentheses for grouping. 
Expressions are evaluated using 64-bit signed integer arithmetic.

### Braced Expression Interpolation

**Preprocessor expressions must be enclosed in curly braces `{}`** to be
evaluated. When the preprocessor encounters a braced expression, it evaluates
the expression and replaces the entire `{expression}` with the resulting value.

Examples:

```asm
; Basic arithmetic - {3 + 2} is replaced with 5
ld r0, {3 + 2}          ; Becomes: ld r0, 5

; Bitwise operations - {0xFF & 0x0F} is replaced with 15
ld r1, {0xFF & 0x0F}    ; Becomes: ld r1, 15

; Complex expressions
ld r2, {(1 + 2) * 3}    ; Becomes: ld r2, 9

; In macro definitions, braced expressions are evaluated at definition time
.define VALUE {1 + 2 * 3}   ; VALUE is defined as 7, not (1 + 2 * 3)
ld r3, VALUE                ; Becomes: ld r3, 7

; Without braces, expressions are NOT evaluated
.define EXPR (1 + 2 * 3)    ; EXPR is defined as literal tokens (1 + 2 * 3)
ld r4, EXPR                 ; Becomes: ld r4, (1 + 2 * 3)
```

This distinction allows developers to choose whether an expression should be
evaluated at preprocessing time (with braces) or preserved for later assembly
stages (without braces).

Expressions are used in various preprocessor directives, such as `.if`, `.elif`,
`.rept`, `.for`, and `.while`, as well as for parameter values in macros. In
these directive contexts, the braces are still required.

### Numeric Literals

The preprocessor supports several numeric literal formats:

- **Decimal**: A sequence of digits (`0-9`), optionally prefixed with a sign
    (`+` or `-`). Example: `42`, `-17`, `+100`.
- **Hexadecimal**: A sequence of hexadecimal digits (`0-9`, `A-F`, `a-f`)
    prefixed with `0x` or `0X`. Example: `0xFF`, `0x1A3B`.
- **Binary**: A sequence of binary digits (`0` or `1`) prefixed with `0b` or
    `0B`. Example: `0b11010110`, `0B1010`.
- **Octal**: A sequence of octal digits (`0-7`) prefixed with `0o` or `0O`.
    Example: `0o755`, `0O177`.
- **Character Literals**: A single character enclosed in single quotes,
    evaluating to its ASCII/Unicode code point. Example: `'A'` (evaluates to
    `65`), `'\n'` (evaluates to `10`).
- **Fixed-Point Literals**: Numeric literals can also include a decimal point
    to represent fixed-point values. Fixed-point literals are represented in
    `32.32` format, in which the upper 32 bits represent the signed integer part,
    and the lower 32 bits represent the fractional part. Example: `3.14`,
    `-0.5`, `2.0`.

### String Literals

String literals are sequences of characters enclosed in double quotes (`"`).
The following escape sequences are supported within string literals:

| Escape Sequence | Description              |
|-----------------|--------------------------|
| `\\`            | Backslash                |
| `\"`            | Double quote             |
| `\'`            | Single quote             |
| `\n`            | Newline (LF)             |
| `\r`            | Carriage return (CR)     |
| `\t`            | Horizontal tab           |
| `\0`            | Null character           |
| `\xNN`          | Hexadecimal byte (2 hex digits) |
| `\uNNNN`        | Unicode code point (4 hex digits) |

### Operator Precedence

Operators in preprocessor expressions are evaluated according to the following
precedence (from highest to lowest):

| Precedence | Operators                     | Associativity |
|------------|-------------------------------|---------------|
| 1 (highest)| `()` (grouping)               | N/A           |
| 2          | `!`, `~`, unary `+`, unary `-`| Right-to-left |
| 3          | `*`, `/`, `%`                 | Left-to-right |
| 4          | `+`, `-`                      | Left-to-right |
| 5          | `<<`, `>>`                    | Left-to-right |
| 6          | `<`, `<=`, `>`, `>=`          | Left-to-right |
| 7          | `==`, `!=`                    | Left-to-right |
| 8          | `&`                           | Left-to-right |
| 9          | `^`                           | Left-to-right |
| 10         | `|`                           | Left-to-right |
| 11         | `&&`                          | Left-to-right |
| 12 (lowest)| `||`                          | Left-to-right |

### Built-in Functions

The preprocessor provides several built-in functions that can be used within
expressions:

#### Numeric Functions

- `fint(FP)`: Returns the integer part of the fixed-point number `FP`.
- `ffrac(FP)`: Returns the fractional part of the fixed-point number `FP`.

#### String Functions

- `strlen(STR)`: Returns the length of the string `STR` in characters.
- `strcmp(STR1, STR2)`: Compares two strings lexicographically. Returns `0`
    if they are equal, a negative value if `STR1` < `STR2`, or a positive
    value if `STR1` > `STR2`.
- `substr(STR, START [, LENGTH])`: Returns a substring of `STR` starting at
    index `START` (0-based). If `LENGTH` is provided, at most `LENGTH`
    characters are returned; otherwise, the rest of the string is returned.
- `indexof(STR, SEARCH)`: Returns the index of the first occurrence of
    `SEARCH` within `STR`, or `-1` if not found.
- `toupper(STR)`: Returns the string `STR` converted to uppercase.
- `tolower(STR)`: Returns the string `STR` converted to lowercase.
- `concat(STR1, STR2, ...)`: Concatenates all provided strings into a single
    string.

#### Miscellaneous Functions

- `defined(NAME)`: Returns `1` if a macro named `NAME` is defined, `0`
    otherwise. This is useful in conditional expressions.
- `typeof(EXPR)`: Returns a string indicating the type of the expression:
    `"integer"`, `"fixed-point"`, `"string"`, or `"undefined"`.

#### Example Usage

```asm
.define MY_STRING "Hello, World!"

.if strlen(MY_STRING) > 10
    .message "String is longer than 10 characters"
.endif

.if defined(DEBUG_MODE)
    .message "Debug mode is enabled"
.endif
```

## Macros

Macros in the G10 assembly preprocessor are a powerful feature that allows for
code reuse and parameterization. The G10 preprocessor supports two types of
macros:

- **Text-Substitution Macros**: Defined using the `.define` directive,
    these macros perform simple text substitution. Whenever the macro name is
    encountered in the source code, it is replaced with the defined replacement
    text. This replacement text begins from the first non-whitespace character
    after the macro name and continues to the end of the line or file.
- **Parameterized Macros**: Defined using the `.macro` directive, these more
    complex macros can accept parameters. The macro body can reference these
    parameters either by name or by positional index. When a parameterized
    macro is invoked, the arguments provided are substituted for the parameters
    in the macro body.

### Macro Naming Rules

Macro names must adhere to the following rules:

- Must begin with a letter (`A-Z`, `a-z`) or underscore (`_`).
- May contain letters, digits (`0-9`), and underscores.
- Are case-sensitive (`MyMacro` and `MYMACRO` are different macros).
- Cannot be a reserved keyword or instruction mnemonic.
- Cannot begin with double underscore (`__`), as such names are reserved for
    built-in macros.

### Built-in Macros

The preprocessor provides several predefined macros that are always available:

| Macro Name       | Description                                              |
|------------------|----------------------------------------------------------|
| `__FILE__`       | The name of the current source file being processed.     |
| `__LINE__`       | The current line number in the source file.              |
| `__DATE__`       | The current date in `"YYYY-MM-DD"` format.               |
| `__TIME__`       | The current time in `"HH:MM:SS"` format.                 |
| `__TIMESTAMP__`  | Combined date and time in ISO 8601 format.               |
| `__COUNTER__`    | A unique integer that increments each time it is used.   |

Example:

```asm
.message "Assembling {__FILE__} at line {__LINE__} on {__DATE__} {__TIME__}"
.message "Unique ID: {__COUNTER__}"

; More on braced expressions and interpolation later...
```

### Text-Substitution Macros

The most basic form of macros in the G10 assembly preprocessor are the
text-substitution macros. These are defined using the `.define` directive, and
take on the following syntax:

```asm
.define MACRO_NAME REPLACEMENT_TEXT
```

Where:
- `MACRO_NAME` is the name of the macro being defined. It must be a valid
    identifier.
- `REPLACEMENT_TEXT` is the text that will replace occurrences of `MACRO_NAME`
    in the source code. This text can include any characters, including
    whitespace, and continues until the end of the line or file.

#### Interpolation

Although text-substitution macros do not support parameters, they can still
leverage dynamic content through the use of interpolation. Within the 
`REPLACEMENT_TEXT`, you can use braced expressions (`{}`) to reference other
simple-text macros or preprocessor expressions. When the macro is expanded, these
interpolated expressions are evaluated and replaced with their corresponding
values.

Examples:

```asm
.define THE_ANSWER 42
.define MESSAGE_STRING "The answer is {THE_ANSWER}."

; ...

.byte MESSAGE_STRING    ; This will expand to: .byte "The answer is 42."
```

```asm
.define MESSAGE_STRING "Value of 2 + 2 is {2 + 2}."

; ...

.byte MESSAGE_STRING    ; This will expand to: .byte "Value of 2 + 2 is 4."
```

#### Line Continuation

When using text-substitution macros, or perhaps even in other contexts, it may be
necessary to split long lines for better readability.

Long lines can be split across multiple physical lines using the backslash (`\`)
character. When a backslash appears at the end of a line (immediately before
the newline character), the backslash and newline are removed, and the next
line is appended to the current line. This is useful for long macro definitions
or complex expressions.

Examples:

```asm
.define LONG_EXPRESSION (VALUE1 + VALUE2 + VALUE3 + \
                         VALUE4 + VALUE5 + VALUE6)
```

```asm
.define OBNOXIOUSLY_LONG_MACRO_NAME This is a very long macro definition that \
    continues onto the next line for better readability. Also, the quick brown \
    fox jumps over the lazy dog.
```

- **Note**: Whitespace after the backslash (before the newline) is not
    permitted and will result in a preprocessor error.

- **Warning**: Line-continuation characters (`\`) are not allowed within
    interpolated expressions. Attempting to use a line-continuation character
    inside `{}` will result in a preprocessor error.

### Parameterized Macros

Parameterized macros are more complex and powerful than simple text-substitution 
macros. These macros can accept parameters, allowing for dynamic code generation 
based on the arguments provided during invocation. Parameterized macros are 
defined using the `.macro` directive, and take on the following syntax:

```asm
.macro MACRO_NAME [PARAM1, PARAM2, ...]
    ; Macro body
.endm               ; or .endmacro
```

Where:
- `MACRO_NAME` is the name of the macro being defined. It must be a valid
    identifier.
- `PARAM1, PARAM2, ...` is an optional, comma-separated list of named
    parameters. These parameters can be referenced within the macro body by
    their names or by their positional index (starting from `1`).

A parameterized macro is invoked by using its name followed by a comma-separated
list of arguments:

```asm
MACRO_NAME ARG1, ARG2, ...
```

Arguments can be any valid preprocessor expressions, and they will be evaluated
and substituted for the corresponding parameters in the macro body during
expansion.

#### Arguments and Parameters

Within the body of a parameterized macro, the arguments passed during invocation
can be referenced in one of two ways:

- **By Name**: A macro invocation argument can be referenced by prefixing its
    parameter name with an "at" symbol (`@`). For example, if a macro has a
    parameter named `VALUE`, it can be referenced within the macro body as
    `@VALUE`.
    - If a parameterized macro is defined with named parameters, it must be
        invoked with at least as many arguments as there are named parameters,
        otherwise the preprocessor will raise an error.
- **By Positional Index**: A macro invocation argument can also be referenced
    by its positional index, starting from `1`. For example, the first argument
    can be referenced as `@1`, the second as `@2`, and so on.

#### Special Parameters

In addition to user-defined parameters, a parameterized macro's body can also 
reference certain special parameters, all also prefixed with an "at" symbol (`@`).
These special parameters are case-insensitive and include the following:

- `@argc`, `@narg`, `@#`: Represents the current number of arguments available to 
    the macro invocation. This is useful for variadic macros, where the number of
    arguments may vary.
- `@argt`: Represents the total number of arguments originally passed into the
    macro invocation, before any `.shift` operations were applied. This is useful
    for tracking the original argument count in variadic macros.
- `@0`: Represents the name of the macro being invoked. This can be useful
    for generating debug messages or logging within the macro body.
- `@!`: Represents all arguments passed to the macro invocation, concatenated
    together as a single string, separated by commas. This is useful for passing
    all remaining arguments to another macro or directive.
- `@*`: Represents all arguments passed to the macro invocation, concatenated
    together as a single string, separated by spaces. This is useful for passing
    all remaining arguments to another macro or directive.
- `@?`: Represents a unique identifier for the current macro invocation. This can
    be useful for generating unique labels or symbols within the macro body to
    avoid naming conflicts.
- `@@`: Represents a single literal "at" symbol (`@`). This is useful for
    situations where you need to include an actual `@` character in the output
    of the macro. For example, you could use `@@1` if you wanted to output the
    string "@1" literally.

#### Referencing Parameters

As mentioned before, parameters in a parameterized macro can be referenced
either by name or by positional index, using the "at" symbol (`@`) prefix. Some
basic examples of a parameterized macro is as follows:

```asm
; Definition of a parameterized macro with two named parameters
.macro ADD_BYTES SRC1, SRC2
    ld l0, {@SRC1}
    add l0, {@SRC2}
.endm

; Invocation
ADD_BYTES 0x10, 0x20        ; Expands to:
                            ;     ld l0, 0x10
                            ;     add l0, 0x20
```

```asm
; Definition of a parameterized macro with variadic arguments
; - Note that this macro does not define any named parameters
.macro DEFINE_DOUBLED_BYTES
    .rept @argt
        .byte {@1 * 2}
        .shift 1                ; We'll talk about this in the next section
    .endr
.endm

; Invocation
DEFINE_DOUBLED_BYTES 1, 2, 3, 4     ; Expands to:
                                    ;     .byte 2
                                    ;     .byte 4
                                    ;     .byte 6
                                    ;     .byte 8
```

A macro invocation's arguments can also be interpolated within the macro's body
using braced expressions (`{}`):

```asm
; Definition of a parameterized macro that uses interpolation within its body
.macro DEFINE_BLOCK NAME, SIZE
    {@NAME}_start:
        .space @SIZE
    {@NAME}_end:
    .define {@NAME}_size @SIZE
.endm

; Invocation
DEFINE_BLOCK BUFFER, 128        ; Expands to:
                                ;     BUFFER_start:
                                ;         .space 128
                                ;     BUFFER_end:
                                ;     .define BUFFER_size 128
```

```asm
; Definition of a parameterized macro that uses interpolation in strings
.macro LOG_MESSAGE LEVEL, MSG
    .message "[{@LEVEL}] {@MSG}"
.endm

; Invocation
LOG_MESSAGE "INFO", "Initialization complete."
                                ; Expands to:
                                ;     .message "[INFO] Initialization complete."
```

```asm
; Definition of a parameterized macro that uses interpolated expressions and
; variadic arguments
.macro PRINT_VALUES             ; Variadic
    .while @argc > 0            ; Continue while there are arguments left
        .message "Value: {@1 * 3}"
        .shift 1
    .endw
.endm

; Invocation
PRINT_VALUES 10, 20, 30         ; Expands to:
                                ;     .message "Value: 30"
                                ;     .message "Value: 60"
                                ;     .message "Value: 90"
```

#### The `.shift` Directive

Within the body of a parameterized macro, the `.shift` directive can be used
to manipulate the list of arguments passed to the macro. The syntax for the
`.shift` directive is as follows:

```asm
.macro MACRO_NAME [PARAM1, PARAM2, ...]
    .shift <count>
    ; Macro body
.endm
```

Where (in addition to the previously defined parameters):
- `<count>` is a non-negative integer literal or an expression that evaluates
    to a non-negative integer. This value indicates how many positions to shift
    the argument list to the left.

The `.shift` directive works as follows:
- The first `<count>` arguments in the current argument list are discarded.
- The remaining arguments are shifted left by `<count>` positions.
- The special parameter `@argc` is updated to reflect the new number of
    arguments available after the shift.
- If `<count>` is greater than or equal to the current number of arguments
    (`@argc`), all arguments are discarded, and `@argc` becomes `0`.
- If `.shift` is called with a `<count>` of `0`, the argument list remains
    unchanged.
- If `.shift` is used in a parameterized macro with named parameters, the
    named parameters will still correspond to their original positions. For
    example, if a macro is defined with parameters `A`, `B`, and `C`, and
    `.shift 1` is called, then `@A` will still refer to the first argument
    passed during invocation, even though it has been shifted out of the
    current argument list.
- Multiple `.shift` directives can be used within the same macro body,
    allowing for progressive shifting of arguments as needed.
- The `.shift` directive can only be used within the body of a parameterized
    macro. Using it outside of a macro body will result in a preprocessor error.

### Macro Recursion

Parameterized macros may invoke themselves recursively, but a maximum recursion
depth is enforced to prevent infinite loops. The default maximum recursion depth
is 256, but this can be configured using the `.pragma max_recursion <depth>`
directive.

Example of a recursive macro:

```asm
.macro SUM_TO_N N, RESULT
    .if @N <= 0
        .define @RESULT 0
    .else
        SUM_TO_N (@N - 1), TEMP_RESULT
        .define @RESULT (TEMP_RESULT + @N)
        .undef TEMP_RESULT
    .endif
.endm
```

### Local Labels in Macros

When using labels within macros, it is important to ensure they are unique
across multiple invocations. The special parameter `@?` provides a unique
identifier for each macro invocation, which can be used to create unique
local labels:

```asm
.macro LOOP_N_TIMES COUNT
loop_start_{@?}:
    ; Loop body
    dec @COUNT
    jnz loop_start_{@?}
loop_end_{@?}:
.endm
```

Note that a label is local so long as it is not declared global or extern using
the `.global` or `.extern` directives.

## Pragmas

The `.pragma` directive allows for setting various preprocessor options or
behaviors. The syntax for the `.pragma` directive is as follows:

```asm
.pragma <name> [args...]
```

Where:
- `<name>` specifies the pragma to set.
- `args` are optional arguments that can be provided depending on the pragma.

The following pragmas are supported by the G10 assembly preprocessor:

- `once`: Ensures that the current file is only included once during the
    assembly process, even if multiple `.include` directives reference it.
- `max_recursion <depth>`: Sets the maximum recursion depth for parameterized
    macros. `<depth>` must be a positive integer.
- `max_include_depth <depth>`: Sets the maximum include depth for file
    inclusions. `<depth>` must be a positive integer.

### Inserted ("Automatic") Pragmas

The preprocessor may automatically insert certain pragmas into the source
code during processing. These inserted pragmas are not visible in the original
source code and should not be explicitly used by the user. These include
the following:

- `.pragma push_file <filename>`: Marks the beginning of a new file inclusion.
    This pragma is automatically inserted when beginning to process a new include
    file, included via the `.include` directive.
- `.pragma pop_file`: Marks the end of a file inclusion. This pragma is
    automatically inserted when finishing processing an include file and
    returning to the previous file.

## Include File Handling

### Include Search Paths

When processing an `.include` directive, the preprocessor searches for the
specified file in the following order:

1. **Quoted includes** (e.g., `.include "myfile.inc"`):
   - The directory containing the current source file.
   - Directories specified via the `-I` command-line option, in order.
   - The current working directory.

### Include Depth Limit

To prevent infinite include recursion, the preprocessor enforces a maximum
include depth. The default limit is 64 levels, but this can be configured
using the `.pragma max_include_depth <depth>` directive.

## Error Handling

The preprocessor provides detailed error messages to help diagnose issues
in the source code. Error messages include:

- The file name and line number where the error occurred.
- A description of the error.
- Context showing the problematic line of code.
- For macro expansion errors, a stack trace showing the macro invocation chain.

### Error Categories

| Category        | Description                                          |
|-----------------|------------------------------------------------------|
| Syntax Error    | Malformed directive or expression syntax.            |
| Undefined Macro | Reference to a macro that has not been defined.      |
| Redefinition    | Attempt to redefine an existing macro (warning).     |
| Include Error   | File not found or circular include detected.         |
| Expression Error| Invalid expression or type mismatch.                 |
| Recursion Error | Maximum recursion or include depth exceeded.         |
| Argument Error  | Wrong number of arguments in macro invocation.       |

## Preprocessor Output

The result of preprocessing is a single, fully-expanded source text with all
directives processed, macros expanded, and conditional blocks resolved. This
output is then passed to the lexer/tokenizer for the next stage of assembly.

The preprocessor preserves file and line information using special `.pragma`
directives which are left for the parser to interpret, allowing for accurate
error reporting and debugging in subsequent stages: `.pragma push_file <filename>` 
and `.pragma pop_file`.

These directives indicate the start and end of file inclusions, respectively,
and help maintain the correct context for line numbers and file names.
