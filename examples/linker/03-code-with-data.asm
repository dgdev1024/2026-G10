; Test 03: Code with Data Section
; Tests: Code section, data section, loading from ROM addresses
;
; This program has initialized data in ROM and code that references it.

; Initialized data in ROM
.org 0x00003000

message_length:
.byte 5

magic_number:
.word 0xCAFE

large_value:
.dword 0x12345678

; Code section
.org 0x00002000

.global main

main:
    ; Load data from ROM using direct addressing
    ld l0, [message_length]     ; Load byte from ROM
    ld w1, [magic_number]       ; Load word from ROM
    ld d2, [large_value]        ; Load dword from ROM
    
    ; Do some operations
    add l0, 1                   ; Increment the length
    
    ; Loop forever
loop:
    jpb nc, loop
