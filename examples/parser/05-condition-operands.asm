; Test 05: Condition Operands
; Tests the parser's ability to parse condition operands.
; Conditions are used with branch and jump instructions.
; The G10 supports 7 conditions: NC, ZS, ZC, CS, CC, VS, VC

.org 0x2000

target:
    nop

; No condition (always/unconditional)
test_no_condition:
    jpb nc, target      ; NC = no condition (always jump)

; Zero flag conditions
test_zero_flag:
    jpb zs, target      ; ZS = zero set (Z=1)
    jpb zc, target      ; ZC = zero clear (Z=0)

; Carry flag conditions
test_carry_flag:
    jpb cs, target      ; CS = carry set (C=1)
    jpb cc, target      ; CC = carry clear (C=0)

; Overflow flag conditions
test_overflow_flag:
    jpb vs, target      ; VS = overflow set (V=1)
    jpb vc, target      ; VC = overflow clear (V=0)

; Test all conditions with JMP instruction
test_jmp_conditions:
    jmp nc, target
    jmp zs, target
    jmp zc, target
    jmp cs, target
    jmp cc, target
    jmp vs, target
    jmp vc, target

; Test all conditions with CALL instruction
test_call_conditions:
    call nc, target
    call zs, target
    call zc, target
    call cs, target
    call cc, target
    call vs, target
    call vc, target

; Test all conditions with RET instruction
test_ret_conditions:
    ret nc
    ret zs
    ret zc
    ret cs
    ret cc
    ret vs
    ret vc
