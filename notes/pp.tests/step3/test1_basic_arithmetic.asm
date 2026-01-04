; Test 1: Basic Arithmetic
; This test verifies basic arithmetic expression evaluation.

.org 0x0000

    ld r0, {3 + 2}
    ld r1, {10 - 3}
    ld r2, {4 * 5}
    ld r3, {20 / 4}
    ld r4, {17 % 5}
    halt

;----------------------------------------------------------------------------
;
; TEST CASE: Basic Arithmetic
;
; This test verifies that basic arithmetic operations (+, -, *, /, %)
; are correctly evaluated within braced expressions.
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 5
; ld r1, 7
; ld r2, 20
; ld r3, 5
; ld r4, 2
; halt
;
;----------------------------------------------------------------------------
