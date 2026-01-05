; Test 4: Fixed-Point Conversion Functions
; This test verifies fint(), round(), ceil(), floor(), trunc().

.org 0x0000

.byte {fint(3.7)}           ; Should be 3
.byte {fint(-2.3)}          ; Should be -2
.byte {round(3.7)}          ; Should be 4
.byte {round(3.2)}          ; Should be 3
.byte {round(-2.7)}         ; Should be -3
.byte {ceil(3.2)}           ; Should be 4
.byte {ceil(-2.7)}          ; Should be -2
.byte {floor(3.7)}          ; Should be 3
.byte {floor(-2.3)}         ; Should be -3
.byte {trunc(3.7)}          ; Should be 3
.byte {trunc(-2.7)}         ; Should be -2

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Fixed-point conversion functions
;
; This test verifies:
; - fint() extracts the integer part
; - round() rounds to nearest (half away from zero)
; - ceil() rounds toward positive infinity
; - floor() rounds toward negative infinity
; - trunc() truncates toward zero
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte 3
; .byte -2
; .byte 4
; .byte 3
; .byte -3
; .byte 4
; .byte -2
; .byte 3
; .byte -3
; .byte 3
; .byte -2
; halt
;
;----------------------------------------------------------------------------
