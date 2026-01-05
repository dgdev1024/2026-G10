; Test 9: Nested conditionals
; This test verifies that nested .if blocks work correctly.

.define OUTER 1
.define INNER 1

.org 0x0000

.if OUTER
    ld r0, 1
    .if INNER
        ld r1, 2
    .endif
    ld r2, 3
.endif

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Nested conditionals
;
; When both OUTER and INNER are true, all code should be included.
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 1
; ld r1, 2
; ld r2, 3
; halt
;
;----------------------------------------------------------------------------
