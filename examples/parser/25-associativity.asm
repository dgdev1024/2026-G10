; Test 25: Expression Associativity Verification
; Explicitly tests left vs right associativity of operators.

.org 0x2000

; LEFT-ASSOCIATIVE operators (parse left-to-right)
; a OP b OP c = (a OP b) OP c

; Addition is left-associative: 10 - 5 - 2 = (10 - 5) - 2 = 5 - 2 = 3
; NOT: 10 - (5 - 2) = 10 - 3 = 7
test_additive_assoc:
    ld d0, 10 - 5 - 2           ; Should be 3, not 7
    ld d0, (10 - 5) - 2         ; Explicitly 3
    ld d0, 10 - (5 - 2)         ; Explicitly 7

; Multiplication is left-associative: 100 / 10 / 2 = (100 / 10) / 2 = 10 / 2 = 5
; NOT: 100 / (10 / 2) = 100 / 5 = 20
test_multiplicative_assoc:
    ld d0, 100 / 10 / 2         ; Should be 5, not 20
    ld d0, (100 / 10) / 2       ; Explicitly 5
    ld d0, 100 / (10 / 2)       ; Explicitly 20

; Shift is left-associative: 256 >> 2 >> 1 = (256 >> 2) >> 1 = 64 >> 1 = 32
; NOT: 256 >> (2 >> 1) = 256 >> 1 = 128
test_shift_assoc:
    ld d0, 256 >> 2 >> 1        ; Should be 32, not 128
    ld d0, (256 >> 2) >> 1      ; Explicitly 32
    ld d0, 256 >> (2 >> 1)      ; Explicitly 128

; Bitwise AND is left-associative
test_and_assoc:
    ld d0, 0xFF & 0xF0 & 0x0F   ; (0xFF & 0xF0) & 0x0F = 0xF0 & 0x0F = 0x00
    ld d0, (0xFF & 0xF0) & 0x0F ; Explicitly 0x00
    ld d0, 0xFF & (0xF0 & 0x0F) ; 0xFF & 0x00 = 0x00 (same result)

; Bitwise XOR is left-associative
test_xor_assoc:
    ld d0, 0xFF ^ 0xF0 ^ 0x0F   ; (0xFF ^ 0xF0) ^ 0x0F = 0x0F ^ 0x0F = 0x00
    ld d0, (0xFF ^ 0xF0) ^ 0x0F ; Explicitly 0x00
    ld d0, 0xFF ^ (0xF0 ^ 0x0F) ; 0xFF ^ 0xFF = 0x00 (same result)

; Bitwise OR is left-associative
test_or_assoc:
    ld d0, 0x10 | 0x20 | 0x40   ; (0x10 | 0x20) | 0x40 = 0x30 | 0x40 = 0x70
    ld d0, (0x10 | 0x20) | 0x40 ; Explicitly 0x70
    ld d0, 0x10 | (0x20 | 0x40) ; 0x10 | 0x60 = 0x70 (same result)

; RIGHT-ASSOCIATIVE operators (parse right-to-left)
; a OP b OP c = a OP (b OP c)

; Exponentiation is right-associative: 2 ** 3 ** 2 = 2 ** (3 ** 2) = 2 ** 9 = 512
; NOT: (2 ** 3) ** 2 = 8 ** 2 = 64
test_exponent_assoc:
    ld d0, 2 ** 3 ** 2          ; Should be 512, not 64
    ld d0, 2 ** (3 ** 2)        ; Explicitly 512
    ld d0, (2 ** 3) ** 2        ; Explicitly 64

; Triple exponentiation
test_triple_exponent:
    ld d0, 2 ** 2 ** 2 ** 2     ; 2 ** (2 ** (2 ** 2)) = 2 ** (2 ** 4) = 2 ** 16 = 65536
    ld d0, 2 ** (2 ** (2 ** 2)) ; Explicitly 65536
    ld d0, ((2 ** 2) ** 2) ** 2 ; ((4) ** 2) ** 2 = 16 ** 2 = 256

; Unary operators are right-associative (prefix)
; --x = -(-x)
test_unary_assoc:
    ld d0, --5                  ; -(-5) = 5
    ld d0, ---5                 ; -(--5) = -(5) = -5
    ld d0, ----5                ; -(---5) = -(-5) = 5
    ld d0, ~~0xFF               ; ~(~0xFF) = 0xFF
    ld d0, ~~~0xFF              ; ~(~~0xFF) = ~0xFF
