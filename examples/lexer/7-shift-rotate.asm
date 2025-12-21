;
; 7-shift-rotate.asm
; Tests shift and rotate instructions.
;

.org 0x1000
shift_test:
    ld d0, 0x12345678
    sla d0
    sra d0
    srl d0
    rla d0
    rl d0
    rra d0
    rr d0