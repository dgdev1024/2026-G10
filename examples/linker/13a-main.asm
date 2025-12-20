; =============================================================================
; Test 13A: Complex Multi-File Project - Main Module
; =============================================================================
; A more complex multi-file test demonstrating a realistic project structure.
; Link with: 13b-utils.asm, 13c-data.asm
; =============================================================================

.extern util_init
.extern util_add_to_sum
.extern util_get_sum
.extern data_values
.extern data_count
.extern shared_accumulator

.global main

.org $80000000
final_result:
    .dword 1
loop_index:
    .dword 1

.org $2000

main:
    ; Initialize stack
    lsp $FFFFFFFC
    
    ; Call initialization routine
    call util_init
    
    ; Load data count
    ld d0, [data_count]
    st [loop_index], d0
    
    ; Get pointer to data values
    ld d1, data_values
    
sum_loop:
    ; Load current value
    ld d0, [d1]
    
    ; Add to running sum
    push d1                     ; Save data pointer
    call util_add_to_sum
    pop d1                      ; Restore data pointer
    
    ; Move to next value (4 bytes per dword)
    inc d1
    inc d1
    inc d1
    inc d1
    
    ; Decrement loop counter
    ld d0, [loop_index]
    dec d0
    st [loop_index], d0
    
    ; Check if done
    cmp l0, 0
    jpb zc, sum_loop            ; Continue if not zero
    
    ; Get final sum
    call util_get_sum
    st [final_result], d0
    
    halt
