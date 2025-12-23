; Test file for .word directive
; Located in: examples/parser/19-test-directive-word.asm

; Word values in ROM
.org 0x4000

; Single word
.word 0x1234

; Multiple words
.word 0x1234, 0x5678, 0x9ABC, 0xDEF0

; Expressions
.word 0x1000, 0x2000, 0x3000

; Word reservation in RAM
.org 0x80000000

; Reserve words
.word 32
.word 16, 8, 4
