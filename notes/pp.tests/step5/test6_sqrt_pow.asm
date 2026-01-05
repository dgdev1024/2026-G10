; Test 6: Math Functions - sqrt(), pow()
; This test verifies sqrt() and pow() functions.

.org 0x0000

.byte {fint(sqrt(4.0))}         ; sqrt(4) = 2
.byte {fint(sqrt(9.0))}         ; sqrt(9) = 3
.byte {fint(sqrt(16.0))}        ; sqrt(16) = 4
.byte {fint(pow(2.0, 3.0))}     ; 2^3 = 8
.byte {fint(pow(3.0, 2.0))}     ; 3^2 = 9
.byte {fint(pow(2.0, 10.0))}    ; 2^10 = 1024
.byte {pow(2, 10)}              ; 2^10 = 1024

halt

;----------------------------------------------------------------------------
;
; TEST CASE: sqrt() and pow() functions
;
; This test verifies:
; - sqrt() returns the square root
; - pow() raises x to the power of y
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte 2
; .byte 3
; .byte 4
; .byte 8
; .byte 9
; .byte 1024
; .byte 1024
; halt
;
;----------------------------------------------------------------------------
