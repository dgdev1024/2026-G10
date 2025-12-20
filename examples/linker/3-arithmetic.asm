; =============================================================================
; Test 3: Arithmetic Operations (Single File)
; =============================================================================
; Tests various arithmetic instructions and demonstrates relocations.
; =============================================================================

; --- RAM Section ---
.org $80000000
operand_a:
    .dword 1                    ; First operand (32-bit)
operand_b:
    .dword 1                    ; Second operand (32-bit)
sum_result:
    .dword 1                    ; Sum result
diff_result:
    .dword 1                    ; Difference result

; --- ROM Section ---
.org $2000

main:
    ; Load operands
    ld d0, 100                  ; D0 = 100
    st [operand_a], d0          ; Store operand A
    
    ld d1, 50                   ; D1 = 50
    st [operand_b], d1          ; Store operand B
    
    ; Perform addition: D0 = D0 + D1
    add d0, d1                  ; D0 = 100 + 50 = 150
    st [sum_result], d0         ; Store sum
    
    ; Reload operand A
    ld d0, [operand_a]          ; D0 = 100
    
    ; Perform subtraction: D0 = D0 - D1
    sub d0, d1                  ; D0 = 100 - 50 = 50
    st [diff_result], d0        ; Store difference
    
    halt
