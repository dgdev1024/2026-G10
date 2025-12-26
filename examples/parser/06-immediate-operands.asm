; Test 06: Immediate Operands
; Tests the parser's ability to parse immediate value operands.
; Immediate operands are literal values embedded directly in instructions.

.org 0x2000

; Decimal integer literals
test_decimal:
    ld d0, 0
    ld d0, 1
    ld d0, 42
    ld d0, 255
    ld d0, 65535
    ld d0, 4294967295

; Hexadecimal integer literals
test_hexadecimal:
    ld d0, 0x0
    ld d0, 0x1
    ld d0, 0xFF
    ld d0, 0xFFFF
    ld d0, 0xFFFFFFFF
    ld d0, 0xDEADBEEF
    ld d0, 0xCAFEBABE

; Binary integer literals
test_binary:
    ld d0, 0b0
    ld d0, 0b1
    ld d0, 0b10101010
    ld d0, 0b11111111
    ld d0, 0b1111111111111111

; Octal integer literals
test_octal:
    ld d0, 0o0
    ld d0, 0o7
    ld d0, 0o77
    ld d0, 0o377

; Character literals
test_character:
    ld l0, 'A'
    ld l0, 'Z'
    ld l0, '0'
    ld l0, '9'
    ld l0, ' '

; Label references as immediate values
some_label:
test_label_ref:
    ld d0, some_label
    ld d0, test_label_ref
