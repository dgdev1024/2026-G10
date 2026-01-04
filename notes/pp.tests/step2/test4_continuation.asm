; Test 4: Macro with Line Continuation
; This test verifies that macro definitions can use line continuation.

.define LONG_VALUE 0xDEAD + \
                   0xBEEF

.org 0x0000

    ld r0, LONG_VALUE
    halt

;----------------------------------------------------------------------------
;
; TEST CASE: Macro with Line Continuation
;
; This test verifies that text-substitution macro definitions can span
; multiple lines using the backslash line continuation character.
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 0xDEAD + 0xBEEF
; halt
;
;----------------------------------------------------------------------------
