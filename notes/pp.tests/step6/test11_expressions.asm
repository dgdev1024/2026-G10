; Test 11: Expressions in conditions
; This test verifies that complex expressions work in conditions.

.define VALUE 10

.org 0x0000

.if VALUE > 5 && VALUE < 20
    ld r0, 1        ; VALUE is between 5 and 20
.endif

.if (VALUE + 5) == 15
    ld r1, 2        ; VALUE + 5 equals 15
.endif

.if !(VALUE == 0)
    ld r2, 3        ; VALUE is not zero
.endif

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Expressions in conditions
;
; This test verifies that comparison operators, logical operators, and
; arithmetic work correctly in conditional expressions.
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 1
; ld r1, 2
; ld r2, 3
; halt
;
;----------------------------------------------------------------------------
