;
; 8-bit-operations.asm
; Tests bit manipulation instructions.
; Note: Bit operations only work with L registers or [DX] indirect addressing.
;

.org 0x1000
bit_test:
    ld l0, 0xFF
    bit 0, l0
    set 1, l0
    res 2, l0
    tog 3, l0