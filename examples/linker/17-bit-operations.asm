; Test 17: Bit Operations
; Tests: BIT (test), SET, RES (reset) instructions
; Syntax: bit N, reg (bit number first, then register)
;
; This program tests bit manipulation operations.

; BSS section in RAM
.org 0x80000000

result:
.dword 1

; Code section
.org 0x00002000

.global main

main:
    ; Test BIT instruction (test if bit is set)
    ld l0, 0b10101010       ; Binary: 10101010
    
    bit 7, l0               ; Test bit 7 (should be set, Z=0)
    jpb zc, bit7_set        ; Jump if zero clear (bit was set)
    jpb nc, fail

bit7_set:
    bit 0, l0               ; Test bit 0 (should be clear, Z=1)
    jpb zs, bit0_clear      ; Jump if zero set (bit was clear)
    jpb nc, fail

bit0_clear:
    ; Test SET instruction (set a bit)
    ld l1, 0b00000000       ; Start with all zeros
    set 0, l1               ; Set bit 0: 00000001
    set 2, l1               ; Set bit 2: 00000101
    set 4, l1               ; Set bit 4: 00010101
    set 7, l1               ; Set bit 7: 10010101
    ; L1 should now be 0x95 (0b10010101)
    
    ; Test RES instruction (reset/clear a bit)
    ld l2, 0b11111111       ; Start with all ones
    res 0, l2               ; Clear bit 0: 11111110
    res 7, l2               ; Clear bit 7: 01111110
    ; L2 should now be 0x7E (0b01111110)
    
    ; Combine operations
    ld l3, 0b00001111       ; Binary: 00001111
    res 0, l3               ; Clear bit 0: 00001110
    res 1, l3               ; Clear bit 1: 00001100
    set 4, l3               ; Set bit 4:   00011100
    set 5, l3               ; Set bit 5:   00111100
    ; L3 should now be 0x3C (0b00111100)
    
    ; Store final result
    ld d0, 0
    mv l0, l3
    st [result], d0
    
    halt

fail:
    ld d0, 0xDEADBEEF
    st [result], d0
    halt
