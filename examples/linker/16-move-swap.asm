; Test 16: Move and Swap Instructions
; Tests: MV, SWAP register operations
;
; This program tests register-to-register moves and swaps.

.org 0x00002000

.global main

main:
    ; Test 8-bit moves
    ld l0, 0xAA
    mv l1, l0              ; L1 = 0xAA
    mv l2, l1              ; L2 = 0xAA
    
    ; Test 16-bit moves
    ld w0, 0x1234
    mv w1, w0              ; W1 = 0x1234
    mv w2, w1              ; W2 = 0x1234
    
    ; Test 32-bit moves
    ld d0, 0xDEADBEEF
    mv d1, d0              ; D1 = 0xDEADBEEF
    mv d2, d1              ; D2 = 0xDEADBEEF
    
    ; Test 8-bit swap
    ld l0, 0x11
    ld l1, 0x22
    swap l0, l1             ; L0 = 0x22, L1 = 0x11
    
    ; Test 16-bit swap
    ld w0, 0xAAAA
    ld w1, 0xBBBB
    swap w0, w1             ; W0 = 0xBBBB, W1 = 0xAAAA
    
    ; Test 32-bit swap
    ld d0, 0x11111111
    ld d1, 0x22222222
    swap d0, d1             ; D0 = 0x22222222, D1 = 0x11111111
    
    ; Verify swap worked by moving results
    mv d3, d0              ; D3 = 0x22222222
    mv d4, d1              ; D4 = 0x11111111
    
    halt

