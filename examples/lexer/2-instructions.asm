;
; 2-instructions.asm
; Tests basic instruction mnemonics with registers.
;

main:
    ld d0, 0
    ld w1, 0xFFFF
    st [d2], d3
    add d0, d1
    sub w0, w1
    nop
    halt