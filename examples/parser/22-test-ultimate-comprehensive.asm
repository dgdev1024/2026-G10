; Ultimate comprehensive parser test
; Demonstrates all implemented parser features
; Located in: examples/parser/22-test-ultimate-comprehensive.asm

; ===== Directives Section =====
.global main, helper, data_section, bss_section
.extern malloc, free, printf

; ===== Program ROM Section =====
.org 0x4000

main:
    ; Setup
    nop
    di
    
    ; Load immediate values
    ld d0, 0x1000
    ld w0, 100
    ld l0, 'A'
    
    ; Register operations
    add d0, d1
    sub w0, w1
    mv d2, d3
    
    ; Direct memory access
    ld d0, [0x80000000]
    st [0x80000004], d0
    
    ; Indirect memory access
    ld d1, 0x80000000
    ld d2, [d1]
    add d2, 0x1
    st [d1], d2
    
    ei
    halt

helper:
    not d0
    inc w0
    dec l0
    nop

; ===== Data Section (ROM) =====
data_section:
    ; Byte data
    .byte 0x01, 0x02, 0x03, 0x04
    .byte "Hello, World!"
    .byte 65, 'B', 0x43
    
    ; Word data
    .word 0x1234, 0x5678
    .word 0xABCD
    
    ; Dword data
    .dword 0x12345678
    .dword 0xDEADBEEF, 0xCAFEBABE
    .dword 0x11111111

; ===== BSS Section (RAM) =====
.org 0x80000000

bss_section:
    ; Reserve space
    .byte 128       ; 128 bytes
    .word 64        ; 64 words (128 bytes)
    .dword 32       ; 32 dwords (128 bytes)

buffer_start:
    .byte 256

buffer_end:
