; Test 7: Hex and Binary Literals in Expressions
; This test verifies that different literal formats work in expressions.

.org 0x0000

    ld r0, {0xFF + 1}           ; Hex: 255 + 1 = 256
    ld r1, {0b1010 + 0b0101}    ; Binary: 10 + 5 = 15
    ld r2, {0xFF & 0b11110000}  ; Mixed: 255 & 240 = 240
    halt

;----------------------------------------------------------------------------
;
; TEST CASE: Hex and Binary Literals in Expressions
;
; This test verifies that hexadecimal and binary literals are correctly
; parsed and evaluated within braced expressions.
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 256
; ld r1, 15
; ld r2, 240
; halt
;
;----------------------------------------------------------------------------
