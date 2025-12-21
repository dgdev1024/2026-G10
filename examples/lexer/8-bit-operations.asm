;
; 8-bit-operations.asm
; Tests bit manipulation instructions.
;

.org 0x1000
bit_test:
    ld d0, 0xFF
    bit 0, d0
    set 1, d0
    res 2, d0
    tog 3, d0