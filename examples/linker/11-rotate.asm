; =============================================================================
; Test 11: Rotate Operations (Single File)
; =============================================================================
; Tests all rotate instructions: RL, RLC, RR, RRC, RLA, RLCA, RRA, RRCA.
; =============================================================================

.org $80000000
rotate_results:
    .fill 8, $00                ; 8 bytes for rotate results

.org $2000

main:
    ld d0, rotate_results       ; D0 = pointer to results
    
    ; Test RLA (rotate left through carry)
    scf                         ; Set carry = 1
    ld l0, $80                  ; L0 = 10000000
    rla                         ; L0 = 00000001, C = 1 (bit 7 shifted into carry, old carry into bit 0)
    st [d0], l0
    inc d0
    
    ; Test RLCA (rotate left circular)
    ld l0, $81                  ; L0 = 10000001
    rlca                        ; L0 = 00000011, C = 1 (bit 7 shifted into carry AND bit 0)
    st [d0], l0
    inc d0
    
    ; Test RL (rotate left through carry)
    scf                         ; Set carry = 1
    ld l1, $40                  ; L1 = 01000000
    rl l1                       ; L1 = 10000001, C = 0
    st [d0], l1
    inc d0
    
    ; Test RLC (rotate left circular)
    ld l2, $AA                  ; L2 = 10101010
    rlc l2                      ; L2 = 01010101, C = 1 (bit 7 moved to bit 0 and carry)
    st [d0], l2
    inc d0
    
    ; Test RRA (rotate right through carry)
    scf                         ; Set carry = 1
    ld l0, $01                  ; L0 = 00000001
    rra                         ; L0 = 10000000, C = 1 (bit 0 shifted into carry, old carry into bit 7)
    st [d0], l0
    inc d0
    
    ; Test RRCA (rotate right circular)
    ld l0, $81                  ; L0 = 10000001
    rrca                        ; L0 = 11000000, C = 1 (bit 0 shifted into carry AND bit 7)
    st [d0], l0
    inc d0
    
    ; Test RR (rotate right through carry)
    ccf                         ; Clear carry = 0
    ld l3, $01                  ; L3 = 00000001
    rr l3                       ; L3 = 00000000, C = 1
    st [d0], l3
    inc d0
    
    ; Test RRC (rotate right circular)
    ld l4, $55                  ; L4 = 01010101
    rrc l4                      ; L4 = 10101010, C = 1 (bit 0 moved to bit 7 and carry)
    st [d0], l4
    
    halt
