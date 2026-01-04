; Test 6: String Interpolation with Macros
; This test verifies that macros can be used in string interpolation.

.define VERSION 42
.define YEAR 2026

.org 0x0000

.byte "Version {VERSION}, Year {YEAR}"

halt

;----------------------------------------------------------------------------
;
; TEST CASE: String Interpolation with Macros
;
; This test verifies that macro references within braced expressions
; inside strings are properly expanded and evaluated.
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte "Version 42, Year 2026"
; halt
;
;----------------------------------------------------------------------------
