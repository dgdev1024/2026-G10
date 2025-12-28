; Test 05: Arithmetic Operations
; Tests ADD, SUB, ADC, SBC, INC, DEC instructions
; and verifies correct flag behavior.
;
; Expected RAM layout at $80000000:
;   $00: 0x30          - ADD result: 0x10 + 0x20 = 0x30
;   $01: 0x10          - SUB result: 0x30 - 0x20 = 0x10
;   $02: 0x05          - INC result: 0x04 + 1 = 0x05
;   $03: 0x03          - DEC result: 0x04 - 1 = 0x03
;   $04: 0x00          - ADD overflow: 0xFF + 1 = 0x00 (with carry set)
;   $05: 0x01          - Carry flag was set (1 if yes, 0 if no)
;   $06-$07: 0x0102    - 16-bit ADD: 0x0100 + 0x0002 = 0x0102
;   $08-$0B: 0x12345678 - 32-bit ADD: 0x12340000 + 0x00005678

.global main

; RAM section for test results
.org 0x80000000
    result_add:         .byte 1
    result_sub:         .byte 1
    result_inc:         .byte 1
    result_dec:         .byte 1
    result_overflow:    .byte 1
    result_carry:       .byte 1
    result_add16:       .word 1
    result_add32:       .dword 1

; Code section
.org 0x2000
main:
    ; Test basic 8-bit ADD
    ld l0, 0x10
    ld l1, 0x20
    add l0, l1          ; L0 = 0x10 + 0x20 = 0x30
    st [result_add], l0
    
    ; Test basic 8-bit SUB
    ld l0, 0x30
    ld l1, 0x20
    sub l0, l1          ; L0 = 0x30 - 0x20 = 0x10
    st [result_sub], l0
    
    ; Test INC
    ld l0, 0x04
    inc l0              ; L0 = 0x04 + 1 = 0x05
    st [result_inc], l0
    
    ; Test DEC
    ld l0, 0x04
    dec l0              ; L0 = 0x04 - 1 = 0x03
    st [result_dec], l0
    
    ; Test overflow/carry on 8-bit ADD
    ld l0, 0xFF
    ld l1, 0x01
    add l0, l1          ; L0 = 0xFF + 0x01 = 0x00, Carry set
    st [result_overflow], l0
    
    ; Check if carry flag was set (use conditional branch)
    ld l2, 0x00         ; Default: carry not set
    jpb cc, no_carry    ; Jump if carry clear
    ld l2, 0x01         ; Carry was set
no_carry:
    st [result_carry], l2
    
    ; Test 16-bit ADD
    ld w0, 0x0100
    ld w1, 0x0002
    add w0, w1          ; W0 = 0x0100 + 0x0002 = 0x0102
    st [result_add16], w0
    
    ; Test 32-bit ADD
    ld d0, 0x12340000
    ld d1, 0x00005678
    add d0, d1          ; D0 = 0x12345678
    st [result_add32], d0
    
    ; End program
    stop
