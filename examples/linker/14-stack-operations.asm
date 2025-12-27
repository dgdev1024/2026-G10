; Test 14: Stack Operations
; Tests: PUSH, POP, stack frame management
;
; This program demonstrates stack operations for register saving.

; BSS section in RAM
.org 0x80000000

result1:
.dword 1
result2:
.dword 1

; Code section
.org 0x00002000

.global main

main:
    ; Initialize some values
    ld d0, 0x11111111
    ld d1, 0x22222222
    ld d2, 0x33333333
    ld d3, 0x44444444
    
    ; Save registers to stack
    push d0
    push d1
    push d2
    push d3
    
    ; Modify registers (simulate function work)
    ld d0, 0xAAAAAAAA
    ld d1, 0xBBBBBBBB
    ld d2, 0xCCCCCCCC
    ld d3, 0xDDDDDDDD
    
    ; Restore registers (in reverse order)
    pop d3
    pop d2
    pop d1
    pop d0
    
    ; D0-D3 should be restored to original values
    ; Store D0 to verify
    st [result1], d0        ; Should be 0x11111111
    st [result2], d3        ; Should be 0x44444444
    
    ; Test nested function calls with stack
    call nc, outer_function
    
    halt

outer_function:
    push d0                 ; Save D0
    push d1                 ; Save D1
    
    ld d0, 100
    ld d1, 200
    call nc, inner_function
    
    pop d1                  ; Restore D1
    pop d0                  ; Restore D0
    ret

inner_function:
    push d0                 ; Save D0
    ld d0, 999
    pop d0                  ; Restore D0
    ret
