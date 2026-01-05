; Test 13: .elif fallback to .else
; This test verifies .else is taken when no conditions match.

.define VERSION 99

.org 0x0000

.if VERSION == 1
    ld r0, 1
.elif VERSION == 2
    ld r0, 2
.elif VERSION == 3
    ld r0, 3
.else
    ld r0, 0        ; Default case
.endif

halt

;----------------------------------------------------------------------------
;
; TEST CASE: .elif fallback to .else
;
; When no .if or .elif conditions match, .else should be taken.
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 0
; halt
;
;----------------------------------------------------------------------------
