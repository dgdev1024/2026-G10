; =============================================================================
; Test 2: Main Entry Point (Single File)
; =============================================================================
; Tests that the linker correctly identifies 'main' as the entry point.
; This should be preferred over 'start' or '_start' when available.
; =============================================================================

.org $80000000
result:
    .dword 1                    ; 32-bit result variable

.org $2000

; This label should NOT be selected as entry point
start:
    nop
    nop

; This label SHOULD be selected as entry point (preferred)
main:
    ld d0, 42                   ; Load answer to life, universe, everything
    st [result], d0             ; Store it
    halt                        ; Done
