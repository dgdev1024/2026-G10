; Test file for .dword directive
; Located in: examples/parser/20-test-directive-dword.asm

; Dword values in ROM
.org 0x4000

; Single dword
.dword 0x12345678

; Multiple dwords
.dword 0x12345678, 0x9ABCDEF0, 0xDEADBEEF

; Expressions
.dword 0x10000000, 0x20000000

; Dword reservation in RAM
.org 0x80000000

; Reserve dwords
.dword 16
.dword 8, 4, 2
