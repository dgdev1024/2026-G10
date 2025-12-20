; =============================================================================
; Test 4: Branching and Loops (Single File)
; =============================================================================
; Tests conditional branching with various conditions.
; Demonstrates JPB (relative jump), JMP, and condition codes.
; =============================================================================

.org $80000000
loop_count:
    .dword 1                    ; Loop counter
final_value:
    .dword 1                    ; Final computed value

.org $2000

main:
    ; Initialize counter to 10
    ld d0, 10
    st [loop_count], d0
    
    ; Initialize accumulator
    ld d1, 0                    ; D1 will accumulate values
    
count_loop:
    ; Load current count
    ld d0, [loop_count]
    
    ; Add count to accumulator (D0 += D1 won't work, so we swap)
    ; D1 += D0 -> store into D1
    add d0, d1                  ; D0 = D0 + D1
    mv d1, d0                   ; D1 = new sum
    
    ; Reload count and decrement
    ld d0, [loop_count]
    dec d0
    st [loop_count], d0
    
    ; Test if counter is zero
    cmp l0, 0                   ; Compare low byte with 0
    jpb zc, count_loop          ; Loop if not zero (zero flag clear)
    
    ; Store final value (should be 55: 10+9+8+7+6+5+4+3+2+1)
    st [final_value], d1
    
done:
    halt

; Alternative test: unconditional jump
test_unconditional:
    jmp done                    ; Always jump to done
