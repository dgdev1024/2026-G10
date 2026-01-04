; Test 3: Bitwise Operations
; This test verifies bitwise operator evaluation.

.org 0x0000

    ld r0, {0xFF & 0x0F}        ; AND: Should be 15
    ld r1, {0xF0 | 0x0F}        ; OR: Should be 255
    ld r2, {0xFF ^ 0x0F}        ; XOR: Should be 240
    ld r3, {~0}                 ; NOT: Should be -1
    ld r4, {1 << 4}             ; Left shift: Should be 16
    ld r5, {256 >> 4}           ; Right shift: Should be 16
    halt

;----------------------------------------------------------------------------
;
; TEST CASE: Bitwise Operations
;
; This test verifies that bitwise operations (&, |, ^, ~, <<, >>)
; are correctly evaluated.
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 15
; ld r1, 255
; ld r2, 240
; ld r3, -1
; ld r4, 16
; ld r5, 16
; halt
;
;----------------------------------------------------------------------------
