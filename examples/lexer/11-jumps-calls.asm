;
; 11-jumps-calls.asm
; Tests jump and call instructions.
;

.org 0x1000
main:
    jmp label1
    jpb zc, label2
    call subroutine
    ret
    reti

label1:
    nop

label2:
    nop

subroutine:
    nop
    ret