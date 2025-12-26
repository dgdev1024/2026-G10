;
; 16-complex.asm
; Tests complex combinations.
;

.org 0x1000
main:
    ld d0, 0x12345678
    ld d1, [data]
    add d0, d1
    cmp l0, 100
    jpb zc, loop
    halt

loop:
    inc d0
    jpb nc, loop

data:
    .dword 42