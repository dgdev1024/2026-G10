; Test 3-WORKING: Literals - Numeric bases, strings, characters (without $ prefix)
; This is a modified version of test 3 that works with the current lexer implementation.

.ORG $4000

test_literals:
    ; Decimal literals
    LD L0, 0            ; Zero
    LD L1, 42           ; Positive integer
    LD L2, 255          ; Byte maximum
    LD W3, 65535        ; Word maximum
    LD D4, 2147483647   ; Large positive integer

    ; Hexadecimal literals (0x prefix only)
    LD L5, 0x00         ; Hex zero
    LD L6, 0xFF         ; Hex byte maximum
    LD W7, 0xABCD       ; Hex word
    LD D8, 0xDEADBEEF   ; Hex dword

    ; Binary literals (0b prefix)
    LD L13, 0b00000000  ; Binary zero
    LD L14, 0b11111111  ; Binary byte maximum
    LD L15, 0b10101010  ; Binary pattern

    ; Octal literals (0o prefix)
    LD W0, 0o777        ; Octal literal
    LD W1, 0o100        ; Octal 64 decimal

    ; Character literals
    LD L0, 'A'          ; ASCII character
    LD L1, 'Z'          ; Another ASCII character
    LD L2, '0'          ; Digit character
    LD L3, ' '          ; Space character

    ; Character literals with escape sequences
    LD L4, '\n'         ; Newline
    LD L5, '\t'         ; Tab
    LD L6, '\r'         ; Carriage return
    LD L7, '\''         ; Single quote
    LD L8, '\"'         ; Double quote
    LD L9, '\\'         ; Backslash
    LD L10, '\0'        ; Null character
    LD L10, '\x41'      ; Hex escape (ASCII 'A')
    LD L11, '\x0A'      ; Hex escape (newline)

string_data:
    ; String literals (for data definitions)
    .BYTE "Hello, World!", 0      ; Null-terminated string
    .BYTE "G10 CPU", 0x00          ; String with explicit null
    .BYTE "Test\nNewline", 0       ; String with escape sequence

    RET
