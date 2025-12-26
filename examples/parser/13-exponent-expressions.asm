; Test 13: Exponentiation Expressions
; Tests the parser's ability to parse exponentiation expressions.
; Exponentiation operator: ** (power) - RIGHT-ASSOCIATIVE

.org 0x2000

; Simple exponentiation
test_exponentiation:
    ld d0, 2 ** 0           ; 2^0 = 1
    ld d0, 2 ** 1           ; 2^1 = 2
    ld d0, 2 ** 8           ; 2^8 = 256
    ld d0, 2 ** 10          ; 2^10 = 1024
    ld d0, 10 ** 2          ; 10^2 = 100
    ld d0, 10 ** 3          ; 10^3 = 1000

; Chained exponentiation (RIGHT-ASSOCIATIVE)
; 2 ** 3 ** 2 parses as 2 ** (3 ** 2) = 2 ** 9 = 512
; NOT as (2 ** 3) ** 2 = 8 ** 2 = 64
test_right_associative:
    ld d0, 2 ** 3 ** 2      ; 2 ** (3 ** 2) = 2 ** 9 = 512
    ld d0, 2 ** 2 ** 3      ; 2 ** (2 ** 3) = 2 ** 8 = 256

; Exponentiation with grouped expressions
test_grouped_exponent:
    ld d0, (2 ** 3)         ; 8
    ld d0, (2 ** 3) ** 2    ; 8 ** 2 = 64 (override right-assoc)
    ld d0, 2 ** (3 ** 2)    ; 2 ** 9 = 512 (explicit right-assoc)

; Exponentiation with unary operators (negative base only)
test_exponent_unary:
    ld d0, -2 ** 2          ; (-2) ** 2 = 4
    ; Note: Negative exponents are not supported in integer arithmetic

; Exponentiation has higher precedence than multiplicative
; 2 * 3 ** 2 parses as 2 * (3 ** 2) = 2 * 9 = 18
test_precedence:
    ld d0, 2 * 3 ** 2       ; 2 * 9 = 18
    ld d0, 3 ** 2 * 2       ; 9 * 2 = 18
    ld d0, 2 + 3 ** 2       ; 2 + 9 = 11
    ld d0, 10 - 2 ** 3      ; 10 - 8 = 2

; Data with exponentiation expressions (in ROM region)
.org 0x2200
exponent_data:
.byte 2 ** 4, 2 ** 7
.word 2 ** 8, 2 ** 15
.dword 2 ** 16, 2 ** 20, 10 ** 6
