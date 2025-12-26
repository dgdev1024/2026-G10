;
; 1-basic-directives.asm
; Tests basic assembler directives and labels.
;

.org 0x1000
start:
    .byte 42
    .word 0x1234
    .dword 0xDEADBEEF

.org 0x2000
data:
    .byte 1, 2, 3