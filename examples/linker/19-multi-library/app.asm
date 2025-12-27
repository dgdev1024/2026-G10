; Test 19: Library Pattern - Main Application
; Tests: Using a library of utility functions
;
; Main application that uses the math and memory libraries.
; Note: mem_* functions use D1=address, L0=size

; BSS section in RAM
.org 0x80000000

.global app_result
app_result:
.dword 1

.global app_buffer
app_buffer:
.dword 4                ; Reserve 4 dwords (16 bytes)

; Code section
.org 0x00002000

; External library functions
.extern math_add
.extern math_sub
.extern math_mul2
.extern math_div2
.extern mem_zero
.extern mem_fill

.global main

main:
    ; Zero the buffer first
    ld d1, app_buffer       ; Buffer address
    ld l0, 16               ; Size in bytes (L0 for mem functions)
    call nc, mem_zero
    
    ; Compute: ((100 + 50) - 25) * 2 / 2 = 125
    
    ; Step 1: 100 + 50 = 150
    ld d0, 100
    ld d1, 50
    call nc, math_add       ; D0 = 150
    
    ; Step 2: 150 - 25 = 125
    ld d1, 25
    call nc, math_sub       ; D0 = 125
    
    ; Step 3: 125 * 2 = 250
    call nc, math_mul2      ; D0 = 250
    
    ; Step 4: 250 / 2 = 125
    call nc, math_div2      ; D0 = 125
    
    ; Store result
    st [app_result], d0
    
    ; Fill buffer with a pattern
    ld d1, app_buffer
    ld l0, 16               ; Size in bytes
    ld l2, 0xAB             ; Fill value
    call nc, mem_fill
    
    halt
