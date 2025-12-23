; Comprehensive test for all data directives
; Located in: examples/parser/21-test-all-data-directives.asm

.global main, data_start, buffer

; Program ROM section
.org 0x4000

main:
    nop
    ld d0, 0x1000
    halt

; Data in ROM
data_start:
    .byte 0x01, 0x02, 0x03, 0x04
    .byte "Test"
    
    .word 0x1234, 0x5678
    .word 0xABCD
    
    .dword 0x12345678
    .dword 0xDEADBEEF, 0xCAFEBABE

; RAM section
.org 0x80000000

buffer:
    .byte 128           ; Reserve 128 bytes
    
buffer_word:
    .word 64            ; Reserve 64 words (128 bytes)
    
buffer_dword:
    .dword 32           ; Reserve 32 dwords (128 bytes)

done:
