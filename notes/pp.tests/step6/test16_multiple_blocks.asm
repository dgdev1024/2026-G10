; Test 16: Multiple consecutive conditionals
; This test verifies that multiple consecutive conditional blocks work.

.define A 1
.define B 0

.org 0x0000

.if A
    ld r0, 1
.endif

.if B
    ld r1, 1        ; Should NOT appear
.endif

.if A
    ld r2, 1
.endif

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Multiple consecutive conditionals
;
; This test verifies that the preprocessor handles multiple independent
; conditional blocks correctly.
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 1
; ld r2, 1
; halt
;
;----------------------------------------------------------------------------
