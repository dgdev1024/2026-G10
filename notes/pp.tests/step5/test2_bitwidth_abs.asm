; Test 2: Integer Functions - bitwidth(), abs()
; This test verifies the bitwidth and absolute value functions.

.org 0x0000

.byte {bitwidth(0)}         ; Should be 0
.byte {bitwidth(1)}         ; Should be 1
.byte {bitwidth(15)}        ; Should be 4 (1111 binary)
.byte {bitwidth(16)}        ; Should be 5 (10000 binary)
.byte {bitwidth(255)}       ; Should be 8
.byte {abs(-42)}            ; Should be 42
.byte {abs(42)}             ; Should be 42
.byte {abs(0)}              ; Should be 0

halt

;----------------------------------------------------------------------------
;
; TEST CASE: bitwidth() and abs() functions
;
; This test verifies:
; - bitwidth() returns the number of bits needed to represent a value
; - abs() returns the absolute value
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte 0
; .byte 1
; .byte 4
; .byte 5
; .byte 8
; .byte 42
; .byte 42
; .byte 0
; halt
;
;----------------------------------------------------------------------------
