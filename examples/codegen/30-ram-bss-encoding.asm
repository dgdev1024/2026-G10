; Test 30: RAM Section (BSS) Encoding
; Tests that RAM sections correctly generate reservations, not data.

; Code section in ROM
.org 0x2000
code_section:
    ld d0, ram_buffer           ; Load address of RAM buffer
    ld d1, ram_counter          ; Load address of counter
    ld d2, ram_large_block      ; Load address of large block
    ret nc

; Data section in ROM (actual data)
.org 0x3000
rom_data:
.byte 1, 2, 3, 4                ; Actual byte values emitted
.word 0x1234, 0x5678            ; Actual word values emitted
.dword 0xDEADBEEF               ; Actual dword value emitted

; RAM section 1: Small allocations
.org 0x80000000
ram_buffer:
.byte 64                        ; Reserve 64 bytes

ram_counter:
.dword 1                        ; Reserve 1 dword (4 bytes)

ram_word_array:
.word 16                        ; Reserve 16 words (32 bytes)

; RAM section 2: Larger allocations
.org 0x80001000
ram_large_block:
.byte 256                       ; Reserve 256 bytes

ram_page:
.dword 256                      ; Reserve 256 dwords (1024 bytes)

; RAM section 3: Stack area
.org 0x80010000
stack_bottom:
.dword 1                        ; Reserve 1 dword as marker

; Gap in address space for stack growth
.org 0x80011000
stack_top:
.dword 1                        ; Reserve 1 dword as marker

; RAM section 4: Heap area
.org 0x80020000
heap_start:
.byte 1                         ; Reserve 1 byte as marker

; Note: In RAM (BSS) sections:
; - .byte N means reserve N bytes (not emit value N)
; - .word N means reserve N words (N * 2 bytes)
; - .dword N means reserve N dwords (N * 4 bytes)
; - No actual data is emitted, only section size is recorded
