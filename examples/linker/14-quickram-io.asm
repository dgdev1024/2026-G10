; =============================================================================
; Test 14: Quick RAM and I/O Port Access (Single File)
; =============================================================================
; Tests LDQ/STQ (Quick RAM at $FFFF0000) and LDP/STP (I/O ports at $FFFFFF00).
; =============================================================================

.org $80000000
regular_var:
    .dword 1

.org $2000

main:
    ; === Quick RAM Access (16-bit offset from $FFFF0000) ===
    
    ; Store to quick RAM using immediate address
    ld l0, $42
    stq [$0100], l0             ; Store to $FFFF0100
    
    ; Load from quick RAM using immediate address
    ldq l1, [$0100]             ; Load from $FFFF0100, L1 = $42
    
    ; Store to quick RAM using register offset
    ld w0, $0200                ; W0 = offset
    ld l0, $AA
    stq [w0], l0                ; Store to $FFFF0200
    
    ; Load from quick RAM using register offset
    ldq l2, [w0]                ; Load from $FFFF0200, L2 = $AA
    
    ; 16-bit quick RAM access
    ld w1, $1234
    stq [$0300], w1             ; Store word to $FFFF0300
    ldq w2, [$0300]             ; Load word from $FFFF0300
    
    ; 32-bit quick RAM access
    ld d0, $DEADBEEF
    stq [$0400], d0             ; Store dword to $FFFF0400
    ldq d1, [$0400]             ; Load dword from $FFFF0400
    
    ; === I/O Port Access (8-bit offset from $FFFFFF00) ===
    
    ; Store to I/O port using immediate address
    ld l0, $55
    stp [$10], l0               ; Store to $FFFFFF10
    
    ; Load from I/O port using immediate address
    ldp l3, [$10]               ; Load from $FFFFFF10, L3 = $55
    
    ; Store to I/O port using register offset
    ld l4, $20                  ; L4 = port offset
    ld l0, $99
    stp [l4], l0                ; Store to $FFFFFF20
    
    ; Load from I/O port using register offset
    ldp l5, [l4]                ; Load from $FFFFFF20, L5 = $99
    
    halt
