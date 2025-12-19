; Test 5: Directives and Data Definitions
; This test verifies proper handling of assembler directives.

.ORG 0x6000              ; Origin directive - set location counter

data_section:
    ; Byte data definitions
    .BYTE 0x12                      ; Single byte
    .BYTE 0x34, 0x56, 0x78          ; Multiple bytes
    .BYTE "Hello", 0                ; String as bytes

    ; Word data definitions (16-bit)
    .WORD 0x1234                    ; Single word
    .WORD 0xABCD, 0xEF01, 0x2345    ; Multiple words

    ; Double-word data definitions (32-bit)
    .DWORD 0x12345678               ; Single dword
    .DWORD 0xDEADBEEF, 0xCAFEBABE   ; Multiple dwords

.ORG 0x7000              ; Change origin

code_section:
    ; Global symbol declaration (exported symbols)
    .GLOBAL main, init, cleanup

    ; External symbol declaration (imported symbols)
    .EXTERN print_string, read_byte

main:
    CALL init
    LD D0, 42
    CALL cleanup
    RET

init:
    NOP
    RET

cleanup:
    NOP
    RET
