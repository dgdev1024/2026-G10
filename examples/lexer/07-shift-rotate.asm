;
; 7-shift-rotate.asm
; Tests shift and rotate instructions.
;

.org 0x1000
shift_test:
    ld l0, 0x12
    sla l0            ; Shift Left Arithmetic (8-bit)
    sra l0            ; Shift Right Arithmetic (8-bit)
    srl l0            ; Shift Right Logical (8-bit)
    rla               ; Rotate Left Accumulator (L0) through carry
    rl l0             ; Rotate Left through carry (8-bit)
    rra               ; Rotate Right Accumulator (L0) through carry
    rr l0             ; Rotate Right through carry (8-bit)