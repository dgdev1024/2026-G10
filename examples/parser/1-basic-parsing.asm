; Test 1: Basic Parsing - Labels, Instructions, and Simple Operands
; This test verifies the parser can handle basic assembly constructs.

.ORG 0x2000

.GLOBAL main, helper

main:
    LD D0, 0            ; Load immediate
    LD W1, [0x8000]     ; Load from address
    LD L2, [D3]         ; Load indirect
    
    CMP D0, D1          ; Compare registers
    JPB ZS, done        ; Conditional jump
    
    CALL helper         ; Call subroutine
    
done:
    HALT

helper:
    INC D0
    RET

data_section:
    .BYTE "Hello", 0
    .WORD 0x1234, 0x5678
    .DWORD 0xDEADBEEF
