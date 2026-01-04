; Test 7: Identifier Interpolation with Macros
; This test verifies that macros can be used in identifier interpolation.

.define IDX 5
.define SUFFIX end

.org 0x0000

label_{IDX}:            ; Should become label_5:
    ld r0, 0
    halt

;----------------------------------------------------------------------------
;
; TEST CASE: Identifier Interpolation with Macros
;
; This test verifies that macro references within braced expressions
; in identifiers are properly expanded and evaluated.
;
; EXPECTED OUTPUT:
; .org 0x0000
; label_5:
; ld r0, 0
; halt
;
;----------------------------------------------------------------------------
