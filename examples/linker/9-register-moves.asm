; =============================================================================
; Test 9: Register Move Operations (Single File)
; =============================================================================
; Tests various register move instructions between different sizes.
; =============================================================================

.org $80000000
test_result:
    .dword 1

.org $2000

main:
    ; Test 8-bit moves
    ld l0, $AA                  ; L0 = $AA
    mv l1, l0                   ; L1 = $AA (copy L0 to L1)
    mv h0, l0                   ; H0 = $AA (copy L0 to high byte of D0)
    mv l2, h0                   ; L2 = $AA (copy H0 to L2)
    
    ; Test 16-bit moves
    ld w0, $1234                ; W0 = $1234
    mv w1, w0                   ; W1 = $1234
    
    ; Test 32-bit moves
    ld d0, $12345678            ; D0 = $12345678
    mv d1, d0                   ; D1 = $12345678
    mv d2, d1                   ; D2 = $12345678
    
    ; Test word-to-dword moves
    ld w3, $ABCD                ; W3 = $ABCD
    mwh d4, w3                  ; D4 high word = $ABCD (D4 = $ABCD0000)
    ld w4, $EF01                ; W4 = $EF01 (D4 = $ABCDEF01)
    
    ; Test extracting high word
    mwl w5, d4                  ; W5 = high word of D4 = $ABCD
    
    ; Store final test value
    st [test_result], d4
    
    halt
