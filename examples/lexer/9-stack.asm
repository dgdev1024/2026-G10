;
; 9-stack.asm
; Tests stack-related instructions.
;

.org 0x1000
stack_test:
    ld d0, 42
    push d0
    pop d1
    lsp d2
    ssp d3