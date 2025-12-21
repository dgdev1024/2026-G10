;
; 13-move-swap.asm
; Tests move and swap instructions.
;

.org 0x1000
move_test:
    mv d0, d1
    mwh d2, d3
    mwl d4, d5
    swap d6