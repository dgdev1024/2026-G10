;
; 3-literals.asm
; Tests various integer literals in different bases.
;

.org 0x1000
decimal:
    .byte 255
    .word 65535
    .dword 4294967295

hex:
    .byte 0xFF
    .word 0xFFFF
    .dword 0xFFFFFFFF

binary:
    .byte 0b11111111
    .word 0b1111111111111111

octal:
    .byte 0o377
    .word 0o177777