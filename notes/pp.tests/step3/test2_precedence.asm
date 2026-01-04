; Test 2: Operator Precedence
; This test verifies that operator precedence is correctly applied.

.org 0x0000

    ld r0, {1 + 2 * 3}          ; Should be 7, not 9
    ld r1, {(1 + 2) * 3}        ; Should be 9
    ld r2, {10 - 2 - 3}         ; Should be 5 (left-to-right)
    ld r3, {2 + 3 * 4 - 5}      ; Should be 9
    halt

;----------------------------------------------------------------------------
;
; TEST CASE: Operator Precedence
;
; This test verifies that operator precedence is correctly applied:
; - Multiplication and division before addition and subtraction
; - Parentheses override precedence
; - Left-to-right associativity for same precedence
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 7
; ld r1, 9
; ld r2, 5
; ld r3, 9
; halt
;
;----------------------------------------------------------------------------
