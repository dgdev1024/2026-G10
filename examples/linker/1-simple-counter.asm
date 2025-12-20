; =============================================================================
; Test 1: Simple Counter Program (Single File)
; =============================================================================
; This is the simplest test case - a single file with ROM and RAM sections.
; It demonstrates:
;   - Basic ROM section (.org $2000)
;   - Basic RAM/BSS section (.org $80000000)
;   - Entry point detection (start label)
;   - Basic instructions (LD, ST, INC, JPB)
; =============================================================================

; --- RAM Section (BSS) ---
.org $80000000
counter:
    .word 1                     ; Reserve 2 bytes for counter variable

; --- ROM Section (Code) ---
.org $2000
start:
    ld w0, 0                    ; Initialize W0 to 0
    st [counter], w0            ; Store 0 into counter

loop:
    ld w0, [counter]            ; Load counter value
    inc w0                      ; Increment counter
    st [counter], w0            ; Store updated counter
    jpb zc, loop                ; Loop until zero (infinite loop)

done:
    halt                        ; Halt the CPU
