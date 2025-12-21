;
; 17-edge-cases.asm
; Tests edge cases like empty lines, multiple spaces.
;

.org 0x1000


label1:
    ld d0, 0

    ld d1, 1

label2: ld d2, 2

    nop


end: