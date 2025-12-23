; Test file for .byte directive
; Located in: examples/parser/18-test-directive-byte.asm

; Byte values in ROM
.org 0x4000

; Single byte
.byte 0x42

; Multiple bytes
.byte 0x12, 0x34, 0x56, 0x78

; String literal (each character becomes a byte)
.byte "Hello, World!"

; Expressions
.byte 0x9A, 0x100, 0xFF

; Mixed
.byte 65, 'B', 0x43

; Byte reservation in RAM
.org 0x80000000

; Reserve bytes
.byte 64
.byte 32, 16, 8
