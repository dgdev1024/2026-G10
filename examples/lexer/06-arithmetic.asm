;
; 6-arithmetic.asm
; Tests arithmetic and logical instructions.
;

.org 0x1000
calc:
    ld d0, 5
    ld d1, 3
    add d0, d1
    adc l0, d1
    sub d0, d1
    sbc l0, d1
    inc d0
    dec d1
    and l0, d1
    or l0, d1
    xor l0, d1
    not d0
    cmp l0, d1