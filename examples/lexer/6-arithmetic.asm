;
; 6-arithmetic.asm
; Tests arithmetic and logical instructions.
;

.org 0x1000
calc:
    ld d0, 5
    ld d1, 3
    add d0, d1
    adc d0, d1
    sub d0, d1
    sbc d0, d1
    inc d0
    dec d1
    and d0, d1
    or d0, d1
    xor d0, d1
    not d0
    cmp d0, d1