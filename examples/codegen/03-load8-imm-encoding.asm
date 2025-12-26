; Test 03: 8-Bit Load Immediate Encoding
; Tests encoding of 8-bit load immediate instructions.
; Format: 0x10X0 LD LX, IMM8 (3 bytes: opcode + imm8)

.org 0x2000

; Test all L registers with immediate values
test_ld_l_imm8:
    ld l0, 0x00                 ; 0x1000 0x00
    ld l1, 0x11                 ; 0x1010 0x11
    ld l2, 0x22                 ; 0x1020 0x22
    ld l3, 0x33                 ; 0x1030 0x33
    ld l4, 0x44                 ; 0x1040 0x44
    ld l5, 0x55                 ; 0x1050 0x55
    ld l6, 0x66                 ; 0x1060 0x66
    ld l7, 0x77                 ; 0x1070 0x77
    ld l8, 0x88                 ; 0x1080 0x88
    ld l9, 0x99                 ; 0x1090 0x99
    ld l10, 0xAA                ; 0x10A0 0xAA
    ld l11, 0xBB                ; 0x10B0 0xBB
    ld l12, 0xCC                ; 0x10C0 0xCC
    ld l13, 0xDD                ; 0x10D0 0xDD
    ld l14, 0xEE                ; 0x10E0 0xEE
    ld l15, 0xFF                ; 0x10F0 0xFF

; Test boundary values
test_ld_l_boundaries:
    ld l0, 0                    ; Minimum value
    ld l0, 127                  ; Max positive signed
    ld l0, 128                  ; Min negative signed (as unsigned)
    ld l0, 255                  ; Maximum value

; Test H registers with immediate values
test_ld_h_imm8:
    ld h0, 0x00                 ; 0x1100 0x00
    ld h1, 0x11                 ; 0x1110 0x11
    ld h7, 0x77                 ; 0x1170 0x77
    ld h15, 0xFF                ; 0x11F0 0xFF
