; Test 01: Basic Code-Only Program
; Tests: Simple code section, main entry point, basic instructions
;
; This is the simplest possible program - just a main label and a loop.

.org 0x00002000

.global main

main:
    nop                     ; Do nothing
    nop                     ; Do nothing again
    jpb nc, main            ; Loop forever (unconditional jump)
