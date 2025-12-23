; Test file for .org directive
; Located in: examples/parser/17-test-directive-org.asm

; Set origin to program ROM
.org 0x4000

; Code at 0x4000
start:
    nop
    halt

; Set origin to RAM
.org 0x80000000

; Data at 0x80000000
buffer:
