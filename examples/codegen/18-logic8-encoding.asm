; Test 18: 8-Bit Logic Instruction Encoding
; Tests encoding of 8-bit logical operations (using L0 accumulator).

.org 0x2000

; AND L0, IMM8 - 0x7000
test_and_imm:
    and l0, 0x00                ; 0x7000 0x00
    and l0, 0x55                ; 0x7000 0x55
    and l0, 0xFF                ; 0x7000 0xFF
    and l0, 0xAA                ; 0x7000 0xAA
    and l0, 0x0F                ; Mask lower nibble
    and l0, 0xF0                ; Mask upper nibble

; AND L0, LY - 0x710Y
test_and_reg:
    and l0, l0                  ; 0x7100
    and l0, l1                  ; 0x7101
    and l0, l7                  ; 0x7107
    and l0, l15                 ; 0x710F

; AND L0, [DY] - 0x720Y
test_and_indirect:
    and l0, [d0]                ; 0x7200
    and l0, [d1]                ; 0x7201
    and l0, [d15]               ; 0x720F

; OR L0, IMM8 - 0x7300
test_or_imm:
    or l0, 0x00                 ; 0x7300 0x00
    or l0, 0x55                 ; 0x7300 0x55
    or l0, 0xFF                 ; 0x7300 0xFF
    or l0, 0x0F                 ; Set lower nibble
    or l0, 0xF0                 ; Set upper nibble

; OR L0, LY - 0x740Y
test_or_reg:
    or l0, l0                   ; 0x7400
    or l0, l1                   ; 0x7401
    or l0, l7                   ; 0x7407
    or l0, l15                  ; 0x740F

; OR L0, [DY] - 0x750Y
test_or_indirect:
    or l0, [d0]                 ; 0x7500
    or l0, [d7]                 ; 0x7507
    or l0, [d15]                ; 0x750F

; XOR L0, IMM8 - 0x7600
test_xor_imm:
    xor l0, 0x00                ; 0x7600 0x00 (no change)
    xor l0, 0x55                ; 0x7600 0x55
    xor l0, 0xFF                ; 0x7600 0xFF (complement)
    xor l0, 0xAA                ; 0x7600 0xAA

; XOR L0, LY - 0x770Y
test_xor_reg:
    xor l0, l0                  ; 0x7700 (clears L0)
    xor l0, l1                  ; 0x7701
    xor l0, l7                  ; 0x7707
    xor l0, l15                 ; 0x770F

; XOR L0, [DY] - 0x780Y
test_xor_indirect:
    xor l0, [d0]                ; 0x7800
    xor l0, [d7]                ; 0x7807
    xor l0, [d15]               ; 0x780F

; NOT LX - 0x79X0
test_not:
    not l0                      ; 0x7900
    not l1                      ; 0x7910
    not l7                      ; 0x7970
    not l15                     ; 0x79F0

; NOT [DX] - 0x7AX0
test_not_indirect:
    not [d0]                    ; 0x7A00
    not [d1]                    ; 0x7A10
    not [d7]                    ; 0x7A70
    not [d15]                   ; 0x7AF0

; CMP L0, IMM8 - 0x7D00
test_cmp_imm:
    cmp l0, 0x00                ; 0x7D00 0x00
    cmp l0, 0x55                ; 0x7D00 0x55
    cmp l0, 0xFF                ; 0x7D00 0xFF

; CMP L0, LY - 0x7E0Y
test_cmp_reg:
    cmp l0, l0                  ; 0x7E00
    cmp l0, l1                  ; 0x7E01
    cmp l0, l15                 ; 0x7E0F

; CMP L0, [DY] - 0x7F0Y
test_cmp_indirect:
    cmp l0, [d0]                ; 0x7F00
    cmp l0, [d7]                ; 0x7F07
    cmp l0, [d15]               ; 0x7F0F

; CPL alias for NOT L0
test_cpl:
    cpl                         ; 0x7900 (same as NOT L0)
