; Test 12: Multiplicative Expressions
; Tests the parser's ability to parse multiplicative expressions.
; Multiplicative operators: * (multiply), / (divide), % (modulo)

.org 0x2000

; Simple multiplication
test_multiplication:
    ld d0, 2 * 3
    ld d0, 10 * 10
    ld d0, 0x10 * 4
    ld d0, 2 * 3 * 4

; Simple division
test_division:
    ld d0, 10 / 2
    ld d0, 100 / 10
    ld d0, 0x100 / 0x10
    ld d0, 1000 / 10 / 2

; Simple modulo
test_modulo:
    ld d0, 10 % 3
    ld d0, 100 % 7
    ld d0, 0xFF % 16
    ld d0, 1000 % 100 % 10

; Mixed multiplicative (left-associative)
; a * b / c parses as (a * b) / c
test_mixed_multiplicative:
    ld d0, 10 * 5 / 2
    ld d0, 100 / 10 * 2
    ld d0, 20 * 3 % 7
    ld d0, 2 * 3 * 4 / 2

; Multiplicative with grouped expressions
test_grouped_multiplicative:
    ld d0, (2 * 3)
    ld d0, (100 / 10)
    ld d0, (2 * 3) * (4 * 5)
    ld d0, (100 / 10) / (5 / 1)

; Multiplicative with unary operators
test_multiplicative_unary:
    ld d0, 10 * -2          ; 10 * (-2) = -20
    ld d0, -10 * 2          ; (-10) * 2 = -20
    ld d0, -10 * -2         ; (-10) * (-2) = 20
    ld d0, 100 / -10        ; 100 / (-10) = -10

; Multiplicative has higher precedence than additive
; 2 + 3 * 4 parses as 2 + (3 * 4) = 14
test_precedence:
    ld d0, 2 + 3 * 4        ; 2 + 12 = 14
    ld d0, 10 - 2 * 3       ; 10 - 6 = 4
    ld d0, 2 * 3 + 4 * 5    ; 6 + 20 = 26
    ld d0, 100 / 10 + 50 / 5 ; 10 + 10 = 20

; Data with multiplicative expressions
.org 0x80000000
multiplicative_data:
.byte 2 * 3, 10 / 2, 17 % 5
.word 0x10 * 0x10, 0x1000 / 0x10
.dword 1000 * 1000, 1000000 / 1000
