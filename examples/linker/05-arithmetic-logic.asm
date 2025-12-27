; Test 05: Arithmetic and Logic Operations
; Tests: ADD, SUB, AND, OR, XOR, shifts, compares
;
; This program exercises various ALU operations.
; Note: ALU operations must use accumulator registers (D0/W0/L0)

.org 0x00002000

.global main

main:
    ; 32-bit arithmetic operations (D0 is accumulator)
    ld d0, 10               ; D0 = 10
    add d0, 5               ; D0 = 15
    sub d0, 3               ; D0 = 12
    
    ld d0, 1000             ; D0 = 1000
    add d0, 234             ; D0 = 1234
    
    ld d0, 0x10000000       ; D0 = 0x10000000
    add d0, 0x02345678      ; D0 = 0x12345678
    
    ; 8-bit Logic operations (L0 is accumulator)
    ld l0, 0xFF             ; L0 = 0xFF
    and l0, 0x0F            ; L0 = 0x0F
    
    ld l0, 0xF0             ; L0 = 0xF0
    or l0, 0x0F             ; L0 = 0xFF
    
    ld l0, 0xAA             ; L0 = 0xAA
    xor l0, 0xFF            ; L0 = 0x55
    
    ; Shift operations (L0)
    ld l0, 0x01             ; L0 = 0x01
    sla l0                  ; L0 = 0x02 (shift left)
    sla l0                  ; L0 = 0x04
    sla l0                  ; L0 = 0x08
    
    ld l0, 0x80             ; L0 = 0x80
    srl l0                  ; L0 = 0x40 (shift right)
    srl l0                  ; L0 = 0x20
    
    ; Compare and branch
    ld l0, 5
    cmp l0, 5               ; Compare L0 with 5
    jpb zs, equal           ; Jump if zero flag set (equal)
    ld l0, 0xFF             ; This should be skipped
    
equal:
    ; Halt
    halt
