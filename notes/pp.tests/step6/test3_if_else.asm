; Test 3: .if / .else branching
; This test verifies that .else works correctly.

.define MODE 0

.org 0x0000

.if MODE
    ld r0, 1        ; MODE is non-zero
.else
    ld r0, 0        ; MODE is zero
.endif

halt

;----------------------------------------------------------------------------
;
; TEST CASE: .if / .else branching
;
; When .if condition is false, the .else branch should be taken.
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 0
; halt
;
;----------------------------------------------------------------------------
