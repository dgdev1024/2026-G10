; Test 15: .ifdef / .else
; This test verifies .ifdef with .else works correctly.

.define FEATURE_A 1

.org 0x0000

.ifdef FEATURE_A
    ld r0, 1        ; Feature A is enabled
.else
    ld r0, 0        ; Feature A is NOT enabled
.endif

.ifdef FEATURE_B
    ld r1, 1        ; Feature B is enabled
.else
    ld r1, 0        ; Feature B is NOT enabled
.endif

halt

;----------------------------------------------------------------------------
;
; TEST CASE: .ifdef / .else
;
; FEATURE_A is defined, so first block takes the .ifdef branch.
; FEATURE_B is NOT defined, so second block takes the .else branch.
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 1
; ld r1, 0
; halt
;
;----------------------------------------------------------------------------
