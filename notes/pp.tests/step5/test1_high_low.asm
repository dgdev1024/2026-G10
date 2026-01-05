; Test 1: Integer Functions - high() and low()
; This test verifies the high() and low() byte extraction functions.

.org 0x0000

.byte {high(0x1234)}        ; Should be 0x12 = 18
.byte {low(0x1234)}         ; Should be 0x34 = 52
.byte {high(0xABCD)}        ; Should be 0xAB = 171
.byte {low(0xABCD)}         ; Should be 0xCD = 205

halt

;----------------------------------------------------------------------------
;
; TEST CASE: high() and low() functions
;
; This test verifies that high() extracts the upper 8 bits and low()
; extracts the lower 8 bits of a 16-bit value.
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte 18
; .byte 52
; .byte 171
; .byte 205
; halt
;
;----------------------------------------------------------------------------
