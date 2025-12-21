;
; 14-interrupts.asm
; Tests interrupt-related instructions.
;

.org 0x1000
int_test:
    int 0
    reti