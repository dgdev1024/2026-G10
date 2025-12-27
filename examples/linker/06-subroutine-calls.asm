; Test 06: Subroutine Calls
; Tests: CALL, RET, stack operations
;
; This program demonstrates subroutine calls within a single file.

; BSS section in RAM
.org 0x80000000

result:
.dword 1

; Code section
.org 0x00002000

.global main

main:
    ; Set up parameters
    ld d0, 10               ; First parameter
    ld d1, 20               ; Second parameter
    
    ; Call the add subroutine
    call nc, add_numbers
    
    ; D0 now contains the result (30)
    st [result], d0         ; Store result
    
    ; Call again with different parameters
    ld d0, 100
    ld d1, 200
    call nc, add_numbers
    
    ; Halt
    halt

; Subroutine: add_numbers
; Input: D0 = first number, D1 = second number
; Output: D0 = sum
add_numbers:
    push d1                 ; Save D1 on stack
    add d0, d1              ; D0 = D0 + D1
    pop d1                  ; Restore D1
    ret                     ; Return to caller

; Subroutine: multiply_by_two
; Input: D0 = number
; Output: D0 = number * 2
multiply_by_two:
    add d0, d0              ; D0 = D0 + D0 = D0 * 2
    ret
