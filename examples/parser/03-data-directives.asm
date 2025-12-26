; Test 03: Data Directives
; Tests the parser's ability to parse data definition directives.
; These directives define byte, word, and dword data in memory.

.org 0x2000

; .byte directive - defines 8-bit data
data_bytes:
.byte 0x00
.byte 0xFF
.byte 0x7F
.byte 255
.byte 0b10101010

; .word directive - defines 16-bit data
data_words:
.word 0x0000
.word 0xFFFF
.word 0x1234
.word 65535
.word 0b1111111100000000

; .dword directive - defines 32-bit data
data_dwords:
.dword 0x00000000
.dword 0xFFFFFFFF
.dword 0x12345678
.dword 4294967295
.dword 0xDEADBEEF

; Multiple values in a single directive (comma-separated)
multi_bytes:
.byte 1, 2, 3, 4, 5

multi_words:
.word 0x100, 0x200, 0x300

multi_dwords:
.dword 0x1000, 0x2000, 0x3000, 0x4000

.org 0x80000000

reserve_bytes:
.byte 12                ; Reserve 12 bytes in RAM
