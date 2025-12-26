;
; 9-stack.asm
; Tests stack-related instructions.
;

.org 0x1000
stack_test:
    ld d0, 42
    push d0
    pop d1
    spo d2      ; Move SP into d2 (read stack pointer)
    spi d3      ; Move d3 into SP (set stack pointer)