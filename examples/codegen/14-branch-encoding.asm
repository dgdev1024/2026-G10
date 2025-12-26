; Test 14: Branch (Relative Jump) Encoding
; Tests encoding of relative branch instructions.
; Format: 0x42X0 JPB X, SIMM16 (4 bytes)

.org 0x2000

; Forward branches
forward_start:
    jpb nc, forward_target      ; Branch forward
    nop
    nop
    nop
forward_target:
    nop

; Backward branches
backward_target:
    nop
backward_start:
    jpb nc, backward_target     ; Branch backward (negative offset)

; All conditions with branches
test_branch_conditions:
    jpb nc, branch_nc_target    ; Condition 0
branch_nc_target:
    jpb zs, branch_zs_target    ; Condition 1
branch_zs_target:
    jpb zc, branch_zc_target    ; Condition 2
branch_zc_target:
    jpb cs, branch_cs_target    ; Condition 3
branch_cs_target:
    jpb cc, branch_cc_target    ; Condition 4
branch_cc_target:
    jpb vs, branch_vs_target    ; Condition 5
branch_vs_target:
    jpb vc, branch_vc_target    ; Condition 6
branch_vc_target:
    nop

; Self-referencing branch (offset = 0)
self_loop:
    jpb nc, self_loop

; Maximum forward/backward distances (within 16-bit signed range)
test_offset_calculation:
    nop
offset_test_start:
    jpb nc, offset_test_end
    nop
    nop
    nop
    nop
offset_test_end:
    jpb nc, offset_test_start   ; Backward branch
