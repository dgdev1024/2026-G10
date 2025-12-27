; Test 20: Expressions and Labels
; Tests: Label arithmetic, address calculations
;
; This program tests using labels in expressions.
; Note: ALU destination must be accumulator (D0).

; Data section in ROM
.org 0x00003000

data_start:
.dword 0x11111111
.dword 0x22222222
.dword 0x33333333
.dword 0x44444444
data_end:

; Size of data section
data_size:
.dword data_end - data_start

; BSS section in RAM
.org 0x80000000

bss_start:
.dword 4                ; Reserve 4 dwords (16 bytes)
bss_end:

; Code section
.org 0x00002000

.global main

main:
    ; Load data section size (calculated at assembly time)
    ld d0, [data_size]      ; Should be 16 (4 dwords * 4 bytes)
    
    ; Calculate size manually for verification
    ; D0 = data_end - data_start = 16
    ld d0, data_end
    ld d1, data_start
    sub d0, d1              ; D0 = data_end - data_start = 16
    
    ; Load from middle of data using offset
    ld d2, [data_start]     ; First dword
    ld d0, data_start
    ld d1, 8                ; Offset to third dword
    add d0, d1              ; D0 = data_start + 8
    ld d3, [d0]             ; Should be 0x33333333
    
    ; Calculate jump table offset (example pattern)
    ; To get jump_table[2], we need: jump_table + 2 * 2 = jump_table + 4
    ld d0, 4                ; Offset = index * sizeof(word) = 2 * 2 = 4
    ld d1, jump_table
    add d0, d1              ; D0 = address of jump_table[2]

    
    halt

; Example jump table
jump_table:
.word handler_0 - jump_table    ; Offset to handler 0
.word handler_1 - jump_table    ; Offset to handler 1  
.word handler_2 - jump_table    ; Offset to handler 2

handler_0:
    nop
    ret

handler_1:
    nop
    nop
    ret

handler_2:
    nop
    nop
    nop
    ret
