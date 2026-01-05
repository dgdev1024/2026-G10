; Test 17: Deep nesting
; This test verifies that deeply nested conditionals work correctly.

.define A 1
.define B 1
.define C 1

.org 0x0000

.if A
    ld r0, 1
    .if B
        ld r1, 2
        .if C
            ld r2, 3
        .else
            ld r2, 0
        .endif
    .else
        ld r1, 0
    .endif
.else
    ld r0, 0
.endif

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Deep nesting
;
; All conditions are true, so all three branches should be taken.
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 1
; ld r1, 2
; ld r2, 3
; halt
;
;----------------------------------------------------------------------------
