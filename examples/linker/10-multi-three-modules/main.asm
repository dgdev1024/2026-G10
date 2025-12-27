; Test 10: Three Modules - Main
; Tests: Complex multi-file linking with three modules
;
; Main orchestrates calls to both math and io modules.

; BSS section in RAM
.org 0x80000000

.global result_value
result_value:
.dword 1

; Code section
.org 0x00002000

; External functions from other modules
.extern add_values
.extern subtract_values
.extern multiply_by_two
.extern store_output

.global main

main:
    ; Compute: ((10 + 20) - 5) * 2 = 50
    
    ; Step 1: Add 10 + 20
    ld d0, 10
    ld d1, 20
    call nc, add_values     ; D0 = 30
    
    ; Step 2: Subtract 5
    ld d1, 5
    call nc, subtract_values    ; D0 = 25
    
    ; Step 3: Multiply by 2
    call nc, multiply_by_two    ; D0 = 50
    
    ; Step 4: Store the result
    st [result_value], d0
    call nc, store_output
    
    halt
