; Test 34: Swap Instruction Encoding
; Tests encoding of SWAP instructions for all register sizes.
; Note: SWAP swaps portions within a register, not between two registers.
; There is no SWP (exchange between registers) instruction in G10.

.org 0x2000

; SWAP nibbles in L register: SWAP LX
; Format: 0x86X0 (swaps upper and lower nibbles)
test_swap_nibbles:
    swap l0                     ; 0x8600
    swap l1                     ; 0x8610
    swap l7                     ; 0x8670
    swap l15                    ; 0x86F0

; SWAP nibbles in memory: SWAP [DX]
; Format: 0x87X0
test_swap_nibbles_indirect:
    swap [d0]                   ; 0x8700
    swap [d7]                   ; 0x8770
    swap [d15]                  ; 0x87F0

; SWAP bytes in W register: SWAP WX
; Format: 0x88X0 (swaps upper and lower bytes)
test_swap_bytes:
    swap w0                     ; 0x8800
    swap w1                     ; 0x8810
    swap w7                     ; 0x8870
    swap w15                    ; 0x88F0

; SWAP words in D register: SWAP DX
; Format: 0x89X0 (swaps upper and lower words)
test_swap_words:
    swap d0                     ; 0x8900
    swap d1                     ; 0x8910
    swap d7                     ; 0x8970
    swap d15                    ; 0x89F0
