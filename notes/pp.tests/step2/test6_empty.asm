; Test 6: Empty Macro Definition
; This test verifies that macros can be defined with no replacement text.

.define EMPTY

.org 0x0000

    ld r0 EMPTY , 42
    halt

;----------------------------------------------------------------------------
;
; TEST CASE: Empty Macro Definition
;
; This test verifies that macros can be defined with no replacement text.
; When expanded, they should effectively disappear from the output.
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 42
; halt
;
;----------------------------------------------------------------------------
