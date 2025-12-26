; Test 13: Jump Instruction Encoding
; Tests encoding of jump instructions with all conditions.

.org 0x2000

; Define targets for jumps
target_near:
    nop
target_far:
    nop

; JMP - Jump (immediate address)
; Format: 0x40X0 JMP X, IMM32 (6 bytes)
test_jmp_imm:
    jmp nc, target_near         ; Condition 0 (no condition)
    jmp zs, target_near         ; Condition 1 (zero set)
    jmp zc, target_near         ; Condition 2 (zero clear)
    jmp cs, target_near         ; Condition 3 (carry set)
    jmp cc, target_near         ; Condition 4 (carry clear)
    jmp vs, target_near         ; Condition 5 (overflow set)
    jmp vc, target_near         ; Condition 6 (overflow clear)

; JMP - Jump (register indirect)
; Format: 0x41XY JMP X, DY (2 bytes)
test_jmp_reg:
    jmp nc, d0                  ; 0x4100
    jmp nc, d1                  ; 0x4101
    jmp nc, d15                 ; 0x410F
    jmp zs, d0                  ; 0x4110
    jmp zc, d5                  ; 0x4125
    jmp cs, d7                  ; 0x4137
    jmp cc, d10                 ; 0x414A
    jmp vs, d12                 ; 0x415C
    jmp vc, d15                 ; 0x416F

; Test unconditional jump alias
test_jmp_unconditional:
    jmp nc, target_far
