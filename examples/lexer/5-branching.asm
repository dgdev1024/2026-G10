;
; 5-branching.asm
; Tests branching instructions and conditions.
;

.org 0x1000
loop:
    ld d0, 0
    inc d0
    cmp d0, 10
    jpb zc, loop
    jpb nc, done

done:
    halt