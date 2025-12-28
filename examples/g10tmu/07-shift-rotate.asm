; Test 07: Shift and Rotate Operations
; Tests SLA, SRA, SRL, RLC, RRC, RL, RR instructions.
; G10 uses Z80-style naming conventions.
;
; Expected RAM layout at $80000000:
;   $00: 0x20          - SLA result: 0x10 << 1 = 0x20
;   $01: 0x40          - SLA by 2: 0x10 << 2 = 0x40
;   $02: 0x08          - SRL result: 0x10 >> 1 = 0x08
;   $03: 0xC0          - SRA result: 0x80 >> 1 = 0xC0 (sign preserved)
;   $04: 0x40          - SRL result: 0x80 >> 1 = 0x40 (no sign preserve)
;   $05: 0x21          - RLC result: 0x90 rotated left = 0x21 (bit 7 -> bit 0)
;   $06: 0xC8          - RRC result: 0x91 rotated right = 0xC8 (bit 0 -> bit 7)
;   $07: 0x01          - Carry flag from SLA of 0x80 (bit 7 shifted out)

.global main

; RAM section for test results
.org 0x80000000
    result_sla1:    .byte 1
    result_sla2:    .byte 1
    result_srl:     .byte 1
    result_sra:     .byte 1
    result_srl_ns:  .byte 1
    result_rlc:     .byte 1
    result_rrc:     .byte 1
    result_carry:   .byte 1

; Code section
.org 0x2000
main:
    ; Test SLA (Shift Left Arithmetic) by 1
    ld l0, 0x10
    sla l0              ; L0 = 0x10 << 1 = 0x20
    st [result_sla1], l0
    
    ; Test SLA by 2
    ld l0, 0x10
    sla l0
    sla l0              ; L0 = 0x10 << 2 = 0x40
    st [result_sla2], l0
    
    ; Test SRL (Shift Right Logical)
    ld l0, 0x10
    srl l0              ; L0 = 0x10 >> 1 = 0x08
    st [result_srl], l0
    
    ; Test SRA (Shift Right Arithmetic - preserves sign bit)
    ld l0, 0x80         ; 0x80 = -128 in signed
    sra l0              ; L0 = 0x80 >> 1 = 0xC0 (sign preserved)
    st [result_sra], l0
    
    ; Compare with SRL (no sign preservation)
    ld l0, 0x80
    srl l0              ; L0 = 0x80 >> 1 = 0x40 (no sign)
    st [result_srl_ns], l0
    
    ; Test RLC (Rotate Left Circular)
    ld l0, 0x90         ; Binary: 1001_0000
    rlc l0              ; RLC -> 0010_0001 = 0x21 (bit 7 goes to bit 0)
    st [result_rlc], l0
    
    ; Test RRC (Rotate Right Circular)
    ld l0, 0x91         ; Binary: 1001_0001
    rrc l0              ; RRC -> 1100_1000 = 0xC8 (bit 0 goes to bit 7)
    st [result_rrc], l0
    
    ; Test carry flag from SLA
    ld l0, 0x80         ; Bit 7 is set
    sla l0              ; Bit 7 shifted into carry, L0 = 0x00
    ld l1, 0x00         ; Default: carry not set
    jpb cc, no_carry    ; Jump if carry clear
    ld l1, 0x01         ; Carry was set
no_carry:
    st [result_carry], l1
    
    ; End program
    stop
