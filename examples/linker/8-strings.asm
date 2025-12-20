; =============================================================================
; Test 8: String Data and Memory Operations (Single File)
; =============================================================================
; Tests .BYTE directive with string data, .FILL, and memory access patterns.
; =============================================================================

.org $80000000
buffer:
    .fill 32, $00               ; 32-byte buffer, initialized to 0

.org $2000

main:
    ; Copy string to buffer using register indirect addressing
    ld d0, hello_string         ; D0 = pointer to source string
    ld d1, buffer               ; D1 = pointer to destination buffer
    
copy_loop:
    ld l0, [d0]                 ; Load byte from source
    st [d1], l0                 ; Store byte to destination
    
    ; Check if null terminator
    cmp l0, 0
    jpb zs, copy_done           ; If zero, we're done
    
    ; Increment pointers
    inc d0
    inc d1
    jpb copy_loop               ; Continue copying
    
copy_done:
    halt

; String data in ROM
hello_string:
    .byte "Hello, G10 World!", 0

goodbye_string:
    .byte "Goodbye!", 0

; Numeric data
data_table:
    .byte $01, $02, $03, $04
    .byte $05, $06, $07, $08
    .word $1234, $5678, $9ABC
    .dword $DEADBEEF
