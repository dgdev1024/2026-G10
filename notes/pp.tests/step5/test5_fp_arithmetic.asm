; Test 5: Fixed-Point Arithmetic Functions
; This test verifies fmul(), fdiv(), fmod().

.define A 2.5
.define B 4.0

.org 0x0000

.byte {fint(fmul(A, B))}        ; 2.5 * 4.0 = 10.0 -> 10
.byte {fint(fdiv(10.0, B))}     ; 10.0 / 4.0 = 2.5 -> 2
.byte {fint(fmod(10.0, 3.0))}   ; 10.0 mod 3.0 = 1.0 -> 1
.byte {fint(fmul(1.5, 3.0))}    ; 1.5 * 3.0 = 4.5 -> 4

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Fixed-point arithmetic functions
;
; This test verifies:
; - fmul() performs fixed-point multiplication
; - fdiv() performs fixed-point division
; - fmod() performs fixed-point modulo
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte 10
; .byte 2
; .byte 1
; .byte 4
; halt
;
;----------------------------------------------------------------------------
