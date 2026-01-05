; Test 14: Loop with identifier interpolation
; This test verifies loops work with identifier interpolation.

.org 0x0000

.for i, 0, 3
label_{i}:
    ld r{i}, {i * 10}
.endfor

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Loop with identifier interpolation
;
; The loop creates labels and register references using the loop variable.
;
; EXPECTED OUTPUT:
; .org 0x0000
; label_0:
; ld r0, 0
; label_1:
; ld r1, 10
; label_2:
; ld r2, 20
; halt
;
;----------------------------------------------------------------------------
