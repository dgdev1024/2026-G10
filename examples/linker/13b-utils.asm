; =============================================================================
; Test 13B: Complex Multi-File Project - Utility Module
; =============================================================================
; Provides utility functions for the main module.
; Link with: 13a-main.asm, 13c-data.asm
; =============================================================================

.extern shared_accumulator

.global util_init
.global util_add_to_sum
.global util_get_sum

; --- RAM Section ---
.org $80000100
running_sum:
    .dword 1
call_count:
    .dword 1

; --- ROM Section ---
.org $3000

; Initialize utility module
util_init:
    push d0
    
    ; Clear running sum
    ld d0, 0
    st [running_sum], d0
    
    ; Clear call count
    st [call_count], d0
    
    ; Clear shared accumulator
    st [shared_accumulator], d0
    
    pop d0
    ret

; Add value in D0 to running sum
util_add_to_sum:
    push d1
    
    ; Load current sum
    ld d1, [running_sum]
    
    ; Add new value
    add d1, d0
    
    ; Store updated sum
    st [running_sum], d1
    
    ; Also update shared accumulator
    st [shared_accumulator], d1
    
    ; Increment call count
    ld d1, [call_count]
    inc d1
    st [call_count], d1
    
    pop d1
    ret

; Get current sum into D0
util_get_sum:
    ld d0, [running_sum]
    ret
