; Test 04: Code with BSS Section
; Tests: Code section, BSS section (uninitialized RAM)
;
; This program reserves space in RAM for variables.

; Uninitialized variables in RAM (BSS)
; In RAM region, .byte N reserves N bytes, .dword N reserves N dwords
.org 0x80000000

counter:
.dword 1                ; Reserve 1 dword (4 bytes) for counter

buffer:
.byte 16                ; Reserve 16 bytes for buffer

result:
.word 1                 ; Reserve 1 word (2 bytes) for result

; Code section
.org 0x00002000

.global main

main:
    ; Initialize counter to zero
    ld d0, 0
    st [counter], d0
    
    ; Initialize result to zero
    ld w1, 0
    st [result], w1
    
loop:
    ; Load counter
    ld d0, [counter]
    
    ; Increment counter
    inc d0
    
    ; Store counter back
    st [counter], d0
    
    ; Loop forever
    jpb nc, loop
