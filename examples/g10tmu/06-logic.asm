; Test 06: Logic Operations
; Tests AND, OR, XOR, NOT instructions.
; Note: G10 ALU instructions use accumulator (L0/W0/D0) as destination.
;
; Expected RAM layout at $80000000:
;   $00: 0x10          - AND result: 0xF0 & 0x1F = 0x10
;   $01: 0xFF          - OR result: 0xF0 | 0x0F = 0xFF
;   $02: 0xA5          - XOR result: 0xFF ^ 0x5A = 0xA5
;   $03: 0x0F          - NOT result: ~0xF0 = 0x0F
;   $04: 0x00          - AND with 0: 0xAB & 0x00 = 0x00 (test zero flag)
;   $05: 0x01          - Zero flag was set (1 if yes)

.global main

; RAM section for test results
.org 0x80000000
    result_and:     .byte 1
    result_or:      .byte 1
    result_xor:     .byte 1
    result_not:     .byte 1
    result_and_z:   .byte 1
    result_zflag:   .byte 1

; Code section
.org 0x2000
main:
    ; Test 8-bit AND (L0 must be destination)
    ld l0, 0xF0
    ld l1, 0x1F
    and l0, l1          ; L0 = 0xF0 & 0x1F = 0x10
    st [result_and], l0
    
    ; Test 8-bit OR
    ld l0, 0xF0
    ld l1, 0x0F
    or l0, l1           ; L0 = 0xF0 | 0x0F = 0xFF
    st [result_or], l0
    
    ; Test 8-bit XOR
    ld l0, 0xFF
    ld l1, 0x5A
    xor l0, l1          ; L0 = 0xFF ^ 0x5A = 0xA5
    st [result_xor], l0
    
    ; Test NOT (bitwise complement)
    ld l0, 0xF0
    not l0              ; L0 = ~0xF0 = 0x0F
    st [result_not], l0
    
    ; Test AND with zero (test zero flag)
    ld l0, 0xAB
    ld l1, 0x00
    and l0, l1          ; L0 = 0xAB & 0x00 = 0x00, Zero flag set
    st [result_and_z], l0
    
    ; Check if zero flag was set
    ld l2, 0x00         ; Default: zero not set
    jpb zc, no_zero     ; Jump if zero clear
    ld l2, 0x01         ; Zero was set
no_zero:
    st [result_zflag], l2
    
    ; End program
    stop
