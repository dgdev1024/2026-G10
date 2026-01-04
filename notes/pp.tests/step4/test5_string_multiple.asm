; Test 5: String Interpolation with Multiple Expressions
; This test verifies multiple interpolations in one string.

.org 0x0000

.byte "Values: {10 + 5}, {20 * 2}, {100 / 4}"

halt

;----------------------------------------------------------------------------
;
; TEST CASE: String Interpolation with Multiple Expressions
;
; This test verifies that multiple braced expressions within a single
; string literal are all evaluated and replaced.
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte "Values: 15, 40, 25"
; halt
;
;----------------------------------------------------------------------------
