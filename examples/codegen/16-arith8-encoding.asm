; Test 16: 8-Bit Arithmetic Encoding
; Tests encoding of 8-bit arithmetic instructions (using L0 accumulator).

.org 0x2000

; ADD L0, IMM8 - 0x5000
test_add_imm:
    add l0, 0x00                ; 0x5000 0x00
    add l0, 0x55                ; 0x5000 0x55
    add l0, 0xFF                ; 0x5000 0xFF

; ADD L0, LY - 0x510Y
test_add_reg:
    add l0, l0                  ; 0x5100
    add l0, l1                  ; 0x5101
    add l0, l7                  ; 0x5107
    add l0, l15                 ; 0x510F

; ADD L0, [DY] - 0x520Y
test_add_indirect:
    add l0, [d0]                ; 0x5200
    add l0, [d1]                ; 0x5201
    add l0, [d15]               ; 0x520F

; ADC L0, IMM8 - 0x5300 (add with carry)
test_adc_imm:
    adc l0, 0x00                ; 0x5300 0x00
    adc l0, 0xFF                ; 0x5300 0xFF

; ADC L0, LY - 0x540Y
test_adc_reg:
    adc l0, l0                  ; 0x5400
    adc l0, l5                  ; 0x5405

; ADC L0, [DY] - 0x550Y
test_adc_indirect:
    adc l0, [d0]                ; 0x5500
    adc l0, [d7]                ; 0x5507

; SUB L0, IMM8 - 0x5600
test_sub_imm:
    sub l0, 0x00                ; 0x5600 0x00
    sub l0, 0x55                ; 0x5600 0x55
    sub l0, 0xFF                ; 0x5600 0xFF

; SUB L0, LY - 0x570Y
test_sub_reg:
    sub l0, l0                  ; 0x5700
    sub l0, l1                  ; 0x5701
    sub l0, l15                 ; 0x570F

; SUB L0, [DY] - 0x580Y
test_sub_indirect:
    sub l0, [d0]                ; 0x5800
    sub l0, [d15]               ; 0x580F

; SBC L0, IMM8 - 0x5900 (subtract with carry/borrow)
test_sbc_imm:
    sbc l0, 0x00                ; 0x5900 0x00
    sbc l0, 0xFF                ; 0x5900 0xFF

; SBC L0, LY - 0x5A0Y
test_sbc_reg:
    sbc l0, l0                  ; 0x5A00
    sbc l0, l7                  ; 0x5A07

; SBC L0, [DY] - 0x5B0Y
test_sbc_indirect:
    sbc l0, [d0]                ; 0x5B00
    sbc l0, [d7]                ; 0x5B07

; INC LX - 0x5CX0
test_inc_l:
    inc l0                      ; 0x5C00
    inc l1                      ; 0x5C10
    inc l7                      ; 0x5C70
    inc l15                     ; 0x5CF0

; INC [DX] - 0x5DX0
test_inc_indirect:
    inc [d0]                    ; 0x5D00
    inc [d7]                    ; 0x5D70
    inc [d15]                   ; 0x5DF0

; DEC LX - 0x5EX0
test_dec_l:
    dec l0                      ; 0x5E00
    dec l1                      ; 0x5E10
    dec l7                      ; 0x5E70
    dec l15                     ; 0x5EF0

; DEC [DX] - 0x5FX0
test_dec_indirect:
    dec [d0]                    ; 0x5F00
    dec [d7]                    ; 0x5F70
    dec [d15]                   ; 0x5FF0
