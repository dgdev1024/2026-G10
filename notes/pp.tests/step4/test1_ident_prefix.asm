; Test 1: Braced Expression at Start of Identifier
; This test verifies that braced expressions can start an identifier.

.org 0x0000

{21 * 2}_start:         ; Should become 42_start:
    ld r0, 0
    halt

;----------------------------------------------------------------------------
;
; TEST CASE: Braced Expression at Start of Identifier
;
; This test verifies that a braced expression at the beginning of an
; identifier is evaluated and concatenated with the suffix.
;
; EXPECTED OUTPUT:
; .org 0x0000
; 42_start:
; ld r0, 0
; halt
;
;----------------------------------------------------------------------------
