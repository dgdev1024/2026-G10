; Test 14: Shift Expressions
; Tests the parser's ability to parse shift expressions.
; Shift operators: << (left shift), >> (right shift)

.org 0x2000

; Left shift (multiply by powers of 2)
test_left_shift:
    ld d0, 1 << 0           ; 1 << 0 = 1
    ld d0, 1 << 1           ; 1 << 1 = 2
    ld d0, 1 << 4           ; 1 << 4 = 16
    ld d0, 1 << 8           ; 1 << 8 = 256
    ld d0, 0xFF << 8        ; 0xFF << 8 = 0xFF00

; Right shift (divide by powers of 2)
test_right_shift:
    ld d0, 256 >> 1         ; 256 >> 1 = 128
    ld d0, 256 >> 4         ; 256 >> 4 = 16
    ld d0, 256 >> 8         ; 256 >> 8 = 1
    ld d0, 0xFF00 >> 8      ; 0xFF00 >> 8 = 0xFF

; Chained shifts (left-associative)
; a << b >> c parses as (a << b) >> c
test_chained_shifts:
    ld d0, 1 << 8 >> 4      ; (1 << 8) >> 4 = 256 >> 4 = 16
    ld d0, 0x100 >> 4 << 2  ; (0x100 >> 4) << 2 = 0x10 << 2 = 0x40

; Shift with grouped expressions
test_grouped_shift:
    ld d0, (1 << 4)
    ld d0, (256 >> 4)
    ld d0, (1 << 4) << 4
    ld d0, 1 << (4 + 4)

; Shift has lower precedence than additive
; 1 + 2 << 3 parses as (1 + 2) << 3 = 3 << 3 = 24
test_precedence_additive:
    ld d0, 1 + 2 << 3       ; (1 + 2) << 3 = 24
    ld d0, 10 << 1 + 2      ; Actually: 10 << (1 + 2)?? No - check
    ld d0, 8 - 4 >> 1       ; (8 - 4) >> 1 = 4 >> 1 = 2

; Shift with multiplicative (multiplicative has higher precedence)
test_precedence_multiplicative:
    ld d0, 2 * 4 << 2       ; (2 * 4) << 2 = 8 << 2 = 32
    ld d0, 1 << 4 * 2       ; Depends on precedence

; Data with shift expressions
.org 0x80000000
shift_data:
.byte 1 << 4, 0x80 >> 4
.word 1 << 8, 0x8000 >> 8
.dword 1 << 16, 1 << 24, 0x80000000 >> 16
