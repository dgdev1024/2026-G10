;
; 20-misc.asm
; Miscellaneous instructions and constructs.
;

.org 0x1000
misc:
    ld d0, 0b10101010
    ld d1, 0o123
    ld d2, 0xABCDEF
    ld d3, 12345
    st [d4], d5
    ld d6, [d7]
    int 5
    reti
    spo d8
    spi d9