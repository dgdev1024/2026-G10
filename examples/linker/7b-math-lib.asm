; =============================================================================
; Test 7B: Multi-File Linking - Math Library
; =============================================================================
; This module provides math functions to be called from other files.
; Must be linked with: 7a-main.asm
; 
; Exported symbols:
;   - add_numbers: Adds two 32-bit values
;   - sub_numbers: Subtracts two 32-bit values
;   - result_storage: Shared data location
; =============================================================================

; Export symbols for other modules
.global add_numbers
.global sub_numbers
.global result_storage

; --- RAM Section ---
.org $80000000
result_storage:
    .dword 1                    ; Shared result storage

; --- ROM Section (Library Code) ---
.org $3000                      ; Different base address than main

; Function: add_numbers
; Input: D0 = first operand, D1 = second operand
; Output: D0 = D0 + D1
add_numbers:
    add d0, d1                  ; D0 = D0 + D1
    st [result_storage], d0     ; Also store in shared location
    ret

; Function: sub_numbers
; Input: D0 = first operand, D1 = second operand
; Output: D0 = D0 - D1
sub_numbers:
    sub d0, d1                  ; D0 = D0 - D1
    st [result_storage], d0     ; Also store in shared location
    ret
