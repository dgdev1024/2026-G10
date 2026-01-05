; Test 16: .rept alias for .repeat
; This test verifies that .rept and .endr work as aliases.

.org 0x0000

.rept 3, i
    .byte {i}
.endr

halt

;----------------------------------------------------------------------------
;
; TEST CASE: .rept alias for .repeat
;
; .rept and .endr are aliases for .repeat and .endrepeat.
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte 0
; .byte 1
; .byte 2
; halt
;
;----------------------------------------------------------------------------
