; =============================================================================
; Test 13C: Complex Multi-File Project - Data Module
; =============================================================================
; Provides shared data for the project.
; Link with: 13a-main.asm, 13b-utils.asm
; =============================================================================

.global data_values
.global data_count
.global shared_accumulator

; --- RAM Section (shared variables) ---
.org $80000200
shared_accumulator:
    .dword 1

; --- ROM Section (constant data) ---
.org $4000

; Number of values in the data array
data_count:
    .dword 5                    ; 5 values

; Array of values to sum
; Sum should be: 10 + 20 + 30 + 40 + 50 = 150
data_values:
    .dword 10
    .dword 20
    .dword 30
    .dword 40
    .dword 50
