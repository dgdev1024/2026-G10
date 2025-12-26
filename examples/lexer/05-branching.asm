;
; 5-branching.asm
; Tests branching instructions and conditions.
;

.org 0x1000
loop:
    ld l0, 0
    inc l0
    cmp l0, 10
    jpb zc, loop
    jpb nc, done

done:
    halt