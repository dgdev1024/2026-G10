; Test 04: Register Operations
; Tests loading immediate values into various register sizes
; and storing them to RAM for verification.
;
; Expected RAM layout at $80000000:
;   $00: 0x12          - L0 (8-bit low)
;   $01: 0x34          - L1 (8-bit low)
;   $02: 0xAB          - H0 (8-bit high, stored as low byte of W0)
;   $03: 0xCD          - H1 (8-bit high)
;   $04-$05: 0x5678    - W2 (16-bit word, little-endian)
;   $06-$07: 0x9ABC    - W3 (16-bit word)
;   $08-$0B: 0xDEADBEEF - D4 (32-bit dword, little-endian)
;   $0C-$0F: 0xCAFEBABE - D5 (32-bit dword)

.global main

; RAM section for test results
.org 0x80000000
    result_l0:  .byte 1
    result_l1:  .byte 1
    result_h0:  .byte 1
    result_h1:  .byte 1
    result_w2:  .word 1
    result_w3:  .word 1
    result_d4:  .dword 1
    result_d5:  .dword 1

; Code section
.org 0x2000
main:
    ; Test 8-bit low registers (Ln)
    ld l0, 0x12
    st [result_l0], l0
    
    ld l1, 0x34
    st [result_l1], l1
    
    ; Test 8-bit high registers (Hn)
    ; Note: Hn is the high byte of Wn (bits 8-15 of Dn)
    ld h0, 0xAB
    ; To store H0, we need to move it to a low register first
    ; or store W0 and check the high byte
    mv l2, h0
    st [result_h0], l2
    
    ld h1, 0xCD
    mv l3, h1
    st [result_h1], l3
    
    ; Test 16-bit word registers (Wn)
    ld w2, 0x5678
    st [result_w2], w2
    
    ld w3, 0x9ABC
    st [result_w3], w3
    
    ; Test 32-bit dword registers (Dn)
    ld d4, 0xDEADBEEF
    st [result_d4], d4
    
    ld d5, 0xCAFEBABE
    st [result_d5], d5
    
    ; End program
    stop
