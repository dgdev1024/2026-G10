; Test 7: Math Functions - log functions
; This test verifies log2(), log10(), and log() functions.

.org 0x0000

.byte {fint(log2(8.0))}         ; log2(8) = 3
.byte {fint(log2(16.0))}        ; log2(16) = 4
.byte {fint(log10(100.0))}      ; log10(100) = 2
.byte {fint(log10(1000.0))}     ; log10(1000) = 3
.byte {fint(log(8.0, 2.0))}     ; log base 2 of 8 = 3
.byte {fint(log(27.0, 3.0))}    ; log base 3 of 27 = 3

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Logarithm functions
;
; This test verifies:
; - log2() returns base-2 logarithm
; - log10() returns base-10 logarithm
; - log() returns logarithm with specified base
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte 3
; .byte 4
; .byte 2
; .byte 3
; .byte 3
; .byte 3
; halt
;
;----------------------------------------------------------------------------
