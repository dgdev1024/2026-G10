; Test 2: Directives - All directive types
; This test verifies the parser handles all assembler directives.

.ORG 0x3000

; Global and external symbols
.GLOBAL start, loop_func, data_start
.EXTERN print_string, read_byte, get_input

start:
    NOP

; Data definitions - bytes
byte_data:
    .BYTE 0x12
    .BYTE 0x34, 0x56, 0x78
    .BYTE "Test String", 0

; Data definitions - words (16-bit)
word_data:
    .WORD 0x1234
    .WORD 0xABCD, 0xEF01, 0x2345

; Data definitions - dwords (32-bit)
dword_data:
    .DWORD 0x12345678
    .DWORD 0xDEADBEEF, 0xCAFEBABE

.ORG 0x4000

loop_func:
    RET

data_start:
    .BYTE 'A', 'B', 'C', 0
