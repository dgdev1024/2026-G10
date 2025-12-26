; Test 12: Stack Instruction Encoding
; Tests encoding of stack manipulation instructions.

.org 0x2000

; LSP - Load Stack Pointer (immediate)
; Format: 0x3500 LSP IMM32 (6 bytes)
test_lsp:
    lsp 0x80000000              ; 0x3500 + 4-byte address
    lsp 0xFFFF0000              ; Stack at top of quick RAM
    lsp 0x80010000              ; Common stack location

; POP - Pop from Stack
; Format: 0x36X0 POP DX (2 bytes)
test_pop:
    pop d0                      ; 0x3600
    pop d1                      ; 0x3610
    pop d2                      ; 0x3620
    pop d7                      ; 0x3670
    pop d15                     ; 0x36F0

; SSP - Store Stack Pointer (to memory)
; Format: 0x3B00 SSP [ADDR32] (6 bytes)
test_ssp:
    ssp [0x80000000]            ; 0x3B00 + 4-byte address
    ssp [0x80001000]

; PUSH - Push to Stack
; Format: 0x3C0Y PUSH DY (2 bytes)
test_push:
    push d0                     ; 0x3C00
    push d1                     ; 0x3C01
    push d2                     ; 0x3C02
    push d7                     ; 0x3C07
    push d15                    ; 0x3C0F

; SPO - Stack Pointer Out (to register)
; Format: 0x3EX0 SPO DX (2 bytes)
test_spo:
    spo d0                      ; 0x3E00
    spo d1                      ; 0x3E10
    spo d7                      ; 0x3E70
    spo d15                     ; 0x3EF0

; SPI - Stack Pointer In (from register)
; Format: 0x3F0Y SPI DY (2 bytes)
test_spi:
    spi d0                      ; 0x3F00
    spi d1                      ; 0x3F01
    spi d7                      ; 0x3F07
    spi d15                     ; 0x3F0F
