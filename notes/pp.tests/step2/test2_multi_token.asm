; Test 2: Multi-Token Replacement
; This test verifies that macros can have multi-token replacement text.

.define LOAD_ZERO ld r0, 0
.define ADD_ONE add r0, 1
.define EXPRESSION (1 + 2 * 3)

.org 0x0000

start:
    LOAD_ZERO
    ADD_ONE
    ld r1, EXPRESSION
    halt

;----------------------------------------------------------------------------
;
; TEST CASE: Multi-Token Replacement
;
; This test verifies that text-substitution macros can have multiple tokens
; in their replacement text. Each macro invocation should be replaced with
; all tokens from the macro definition.
;
; EXPECTED OUTPUT:
; .org 0x0000
; start:
; ld r0, 0
; add r0, 1
; ld r1, (1 + 2 * 3)
; halt
;
;----------------------------------------------------------------------------
