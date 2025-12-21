;
; 12-memory-ops.asm
; Tests memory operations.
;

.org 0x1000
mem_test:
    ld d0, [0x80000000]
    st [0x80000004], d1
    ldq d2, [d3]
    stq [d4], d5
    ldp d6, [d7]
    stp [d8], d9