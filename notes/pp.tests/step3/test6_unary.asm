; Test 6: Unary Operators
; This test verifies unary operator evaluation.

.org 0x0000
    ld r0, {-42}                ; Unary minus
    ld r1, {+42}                ; Unary plus
    ld r2, {--10}               ; Double negation (should be 10)
    ld r3, {~0xFF}              ; Bitwise NOT of 255
    halt

;----------------------------------------------------------------------------
;
; TEST CASE: Unary Operators
;
; This test verifies that unary operators (+, -, ~) are correctly evaluated.
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, -42
; ld r1, 42
; ld r2, 10
; ld r3, -256
; halt
;
;----------------------------------------------------------------------------
