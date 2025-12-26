; Test 31: Branch Offset Calculation
; Tests that relative branch offsets are correctly calculated.
; Branch offset is relative to the instruction AFTER the branch.

.org 0x2000

; Forward branch (positive offset)
forward_branch_test:
    jpb nc, forward_target_1    ; Offset = +2 (target - (PC after branch))
    nop                         ; This is at PC after branch
forward_target_1:
    nop                         ; Target

; Forward branch with more distance
forward_branch_far:
    jpb nc, forward_target_2    ; Offset = +10
    nop                         ; +0
    nop                         ; +2
    nop                         ; +4
    nop                         ; +6
    nop                         ; +8
forward_target_2:
    nop                         ; Target at +10

; Backward branch (negative offset)
backward_target_1:
    nop
backward_branch_test:
    jpb nc, backward_target_1   ; Negative offset

; Backward branch with more distance
backward_target_2:
    nop                         ; -10
    nop                         ; -8
    nop                         ; -6
    nop                         ; -4
    nop                         ; -2
backward_branch_far:
    jpb nc, backward_target_2   ; Offset = -10

; Self-referencing branch (offset = 0, but actually -4 for branch length)
self_branch:
    jpb nc, self_branch         ; Branch to itself (infinite loop)

; Branch to immediately following instruction
branch_next:
    jpb nc, next_instr          ; Offset = 0 (branch past itself)
next_instr:
    nop

; Conditional branches with same offsets
cond_branch_targets:
    jpb zs, cond_target         ; Zero set
    jpb zc, cond_target         ; Zero clear (different path)
    jpb cs, cond_target         ; Carry set
    jpb cc, cond_target         ; Carry clear
cond_target:
    nop

; Multiple labels at branch targets
multi_target_1:
multi_target_2:
    nop
branch_to_multi:
    jpb nc, multi_target_1      ; Branch to first of multiple labels
    jpb nc, multi_target_2      ; Same address, different label
