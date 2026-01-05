; Test 12: Using defined() function in conditions
; This test verifies that defined() works in .if conditions.

.define FEATURE_A 1

.org 0x0000

.if defined(FEATURE_A)
    ld r0, 1
.endif

.if !defined(FEATURE_B)
    ld r1, 2
.endif

.if defined(FEATURE_A) && !defined(FEATURE_B)
    ld r2, 3
.endif

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Using defined() function in conditions
;
; This test verifies that the defined() built-in function works in
; conditional expressions along with logical operators.
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 1
; ld r1, 2
; ld r2, 3
; halt
;
;----------------------------------------------------------------------------
