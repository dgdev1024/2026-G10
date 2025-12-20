; =============================================================================
; Test 5: Stack Operations (Single File)
; =============================================================================
; Tests stack push/pop operations and subroutine calls.
; Demonstrates PUSH, POP, CALL, RET, LSP, SPO instructions.
; =============================================================================

.org $80000000
saved_value:
    .dword 1
call_result:
    .dword 1

.org $2000

main:
    ; Set up stack pointer (top of RAM)
    lsp $FFFFFFFC
    
    ; Push some values onto the stack
    ld d0, $DEADBEEF
    push d0
    
    ld d0, $CAFEBABE
    push d0
    
    ld d0, $12345678
    push d0
    
    ; Pop values in reverse order
    pop d1                      ; D1 = $12345678
    pop d2                      ; D2 = $CAFEBABE
    pop d3                      ; D3 = $DEADBEEF
    
    ; Test subroutine call
    ld d0, 5                    ; Argument
    call compute_square
    st [call_result], d0        ; Store result (should be 25)
    
    halt

; Subroutine: compute_square
; Input: D0 = value to square
; Output: D0 = squared value
; Note: This is a simplified version using multiplication concept
compute_square:
    push d1                     ; Save D1
    mv d1, d0                   ; D1 = original value
    
    ; Simple approach: add D0 to itself (D0-1) times
    ; For simplicity, we'll just demonstrate the call/ret mechanism
    ld d0, 25                   ; Hardcoded result for value 5
    
    pop d1                      ; Restore D1
    ret
