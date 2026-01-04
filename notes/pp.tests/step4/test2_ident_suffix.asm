; Test 2: Braced Expression at End of Identifier
; This test verifies that braced expressions can end an identifier.

.org 0x0000

label_{50 / 2}:         ; Should become label_25:
    ld r0, 0
    halt

;----------------------------------------------------------------------------
;
; TEST CASE: Braced Expression at End of Identifier
;
; This test verifies that a braced expression at the end of an
; identifier is evaluated and concatenated with the prefix.
;
; EXPECTED OUTPUT:
; .org 0x0000
; label_25:
; ld r0, 0
; halt
;
;----------------------------------------------------------------------------
