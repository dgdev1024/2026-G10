; Test 02: Code with Immediate Values
; Tests: Loading immediate values of various sizes into registers
;
; This program loads various immediate values into registers.

.org 0x00002000

.global main

main:
    ; Load 8-bit immediate values
    ld l0, 0x42             ; Load byte into low register
    ld l1, 0xFF             ; Load max byte value
    
    ; Load 16-bit immediate values
    ld w0, 0x1234           ; Load word into W0
    ld w1, 0xABCD           ; Load word into W1
    
    ; Load 32-bit immediate values
    ld d0, 0x12345678       ; Load dword into D0
    ld d1, 0xDEADBEEF       ; Load dword into D1
    
    ; Halt the CPU
    halt
