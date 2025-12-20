; =============================================================================
; Test 12: Swap Operations (Single File)
; =============================================================================
; Tests SWAP instructions for nibbles, bytes, and words.
; =============================================================================

.org $80000000
swap_results:
    .fill 16, $00               ; Space for swap results

.org $2000

main:
    ld d4, swap_results         ; D4 = pointer to results
    
    ; Test SWAP nibbles in 8-bit register
    ; SWAP LX: exchanges upper nibble and lower nibble
    ld l0, $AB                  ; L0 = $AB (1010_1011)
    swap l0                     ; L0 = $BA (1011_1010)
    st [d4], l0
    inc d4
    
    ld l1, $12                  ; L1 = $12
    swap l1                     ; L1 = $21
    st [d4], l1
    inc d4
    
    ; Test SWAP bytes in 16-bit register
    ; SWAP WX: exchanges upper byte and lower byte
    ld w0, $ABCD                ; W0 = $ABCD
    swap w0                     ; W0 = $CDAB
    ; Store result (2 bytes)
    st [d4], l0                 ; Store low byte
    inc d4
    st [d4], h0                 ; Store high byte
    inc d4
    
    ld w1, $1234                ; W1 = $1234
    swap w1                     ; W1 = $3412
    st [d4], l1                 ; Store low byte
    inc d4
    st [d4], h1                 ; Store high byte
    inc d4
    
    ; Test SWAP words in 32-bit register
    ; SWAP DX: exchanges upper word and lower word
    ld d0, $12345678            ; D0 = $12345678
    swap d0                     ; D0 = $56781234
    st [d4], d0                 ; Store full dword
    
    halt
