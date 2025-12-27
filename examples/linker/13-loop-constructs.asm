; Test 13: Loop Constructs
; Tests: Various loop patterns (while, for, do-while equivalents)
;
; This program demonstrates common loop patterns.
; Note: CMP uses L0. Conditions: nc=always, zs/zc=zero, cs/cc=carry

; BSS section in RAM
.org 0x80000000

sum:
.dword 1

; Code section
.org 0x00002000

.global main

main:
    ; Initialize sum to 0
    ld d0, 0
    st [sum], d0

    ; Calculate sum of 1 to 10 using a while-style loop
    ; L0 = counter (1 to 10)
    
    ld l0, 1                ; counter = 1
    ld d0, 0
    st [sum], d0            ; sum = 0

while_loop:
    ; Check condition: counter > 10?
    ; If L0 - 11 sets carry (L0 < 11), continue. If carry clear (L0 >= 11), exit.
    cmp l0, 11
    jpb cc, while_end       ; If L0 >= 11 (carry clear), exit loop
    
    ; sum += counter (use D0 as accumulator)
    ld d0, [sum]
    ld d1, 0
    mv l1, l0              ; D1 = counter (32-bit)
    add d0, d1              ; D0 = sum + counter
    st [sum], d0            ; Store new sum
    
    ; Increment counter
    inc l0
    
    ; Loop back
    jpb nc, while_loop

while_end:
    ; Load final result - should be 55
    ld d0, [sum]

    ; Count down from 5 to 1 using do-while style
    ld l0, 5                ; counter = 5
    ld d0, 0
    st [sum], d0            ; reset sum

do_while_loop:
    ; sum += counter
    ld d0, [sum]
    ld d1, 0
    mv l1, l0              ; D1 = counter
    add d0, d1
    st [sum], d0
    
    ; Decrement counter
    dec l0
    
    ; Check condition: counter != 0
    cmp l0, 0
    jpb zc, do_while_loop   ; If counter != 0 (zero clear), continue

    ; Final result - sum should be 15 (5+4+3+2+1)
    ld d0, [sum]
    
    halt
