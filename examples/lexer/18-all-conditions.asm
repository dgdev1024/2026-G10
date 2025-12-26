;
; 18-all-conditions.asm
; Tests all branching conditions.
;

.org 0x1000
test_conditions:
    ld l0, 0
    cmp l0, 1
    jpb nc, label_nc
    jpb zs, label_zs
    jpb zc, label_zc
    jpb cs, label_cs
    jpb cc, label_cc
    jpb vs, label_vs
    jpb vc, label_vc

label_nc: nop
label_zs: nop
label_zc: nop
label_cs: nop
label_cc: nop
label_vs: nop
label_vc: nop