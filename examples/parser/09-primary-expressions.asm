; Test 09: Primary Expressions
; Tests the parser's ability to parse primary expressions.
; Primary expressions are the atomic building blocks: literals, identifiers.

.org 0x2000

; Integer literals in expressions
test_integer_literals:
    ld d0, 42               ; Decimal
    ld d0, 0xFF             ; Hexadecimal
    ld d0, 0b1010           ; Binary
    ld d0, 0o77             ; Octal

; Character literals
test_char_literals:
    ld l0, 'A'
    ld l0, 'z'
    ld l0, '0'

; Identifier (label) references
some_label:
another_label:
test_identifiers:
    ld d0, some_label
    ld d0, another_label
    jmp nc, some_label

; Grouped expressions (parentheses)
test_grouped:
    ld d0, (42)
    ld d0, (0xFF)
    ld d0, ((100))
    ld d0, (((1)))

; Data directives with primary expressions
.org 0x80000000
primary_data:
.byte 10, 20, 30
.word 0x1234, 0x5678
.dword some_label, another_label
