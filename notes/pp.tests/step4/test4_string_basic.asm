; Test 4: String Interpolation Basic
; This test verifies basic string interpolation.

.org 0x0000

.byte "The answer is {2 + (10 * 4)}."

halt

;----------------------------------------------------------------------------
;
; TEST CASE: String Interpolation Basic
;
; This test verifies that braced expressions within string literals are
; evaluated and replaced with their results.
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte "The answer is 42."
; halt
;
;----------------------------------------------------------------------------
