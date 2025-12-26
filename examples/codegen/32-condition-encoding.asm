; Test 32: Condition Code Encoding
; Tests that all 7 condition codes are correctly encoded in instructions.
; Condition codes: NC=0, ZS=1, ZC=2, CS=3, CC=4, VS=5, VC=6

.org 0x2000

; Target for all jumps/branches
target:
    nop

; JMP with all conditions (0x40X0)
test_jmp_conditions:
    jmp nc, target              ; 0x4000 - condition 0
    jmp zs, target              ; 0x4010 - condition 1
    jmp zc, target              ; 0x4020 - condition 2
    jmp cs, target              ; 0x4030 - condition 3
    jmp cc, target              ; 0x4040 - condition 4
    jmp vs, target              ; 0x4050 - condition 5
    jmp vc, target              ; 0x4060 - condition 6

; JMP register with all conditions (0x41XY)
test_jmp_reg_conditions:
    jmp nc, d0                  ; 0x4100
    jmp zs, d0                  ; 0x4110
    jmp zc, d0                  ; 0x4120
    jmp cs, d0                  ; 0x4130
    jmp cc, d0                  ; 0x4140
    jmp vs, d0                  ; 0x4150
    jmp vc, d0                  ; 0x4160

; JPB (branch) with all conditions (0x42X0)
test_jpb_conditions:
    jpb nc, target              ; 0x4200 - condition 0
    jpb zs, target              ; 0x4210 - condition 1
    jpb zc, target              ; 0x4220 - condition 2
    jpb cs, target              ; 0x4230 - condition 3
    jpb cc, target              ; 0x4240 - condition 4
    jpb vs, target              ; 0x4250 - condition 5
    jpb vc, target              ; 0x4260 - condition 6

; CALL with all conditions (0x43X0)
test_call_conditions:
    call nc, target             ; 0x4300 - condition 0
    call zs, target             ; 0x4310 - condition 1
    call zc, target             ; 0x4320 - condition 2
    call cs, target             ; 0x4330 - condition 3
    call cc, target             ; 0x4340 - condition 4
    call vs, target             ; 0x4350 - condition 5
    call vc, target             ; 0x4360 - condition 6

; Note: CALL with register operand is not in the spec.
; Only CALL X, IMM32 exists (opcode 0x43X0).

; RET with all conditions (0x45X0)
test_ret_conditions:
    ret nc                      ; 0x4500 - condition 0
    ret zs                      ; 0x4510 - condition 1
    ret zc                      ; 0x4520 - condition 2
    ret cs                      ; 0x4530 - condition 3
    ret cc                      ; 0x4540 - condition 4
    ret vs                      ; 0x4550 - condition 5
    ret vc                      ; 0x4560 - condition 6

; RETI with all conditions (0x46X0)
test_reti_conditions:
    reti nc                     ; 0x4600 - condition 0
    reti zs                     ; 0x4610 - condition 1
    reti zc                     ; 0x4620 - condition 2
    reti cs                     ; 0x4630 - condition 3
    reti cc                     ; 0x4640 - condition 4
    reti vs                     ; 0x4650 - condition 5
    reti vc                     ; 0x4660 - condition 6
