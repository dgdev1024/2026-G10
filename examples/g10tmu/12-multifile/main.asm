; Test 12: Multi-file Project - Main Module
; This is the main entry point that calls functions from the library module.
;
; Expected RAM layout at $80000000:
;   $00: 0x0A         - Result from add_numbers(3, 7) = 10
;   $01: 0x15         - Result from multiply_by_three(7) = 21
;   $02: 0x10         - Result from double_value(8) = 16
;   $03: 0x2A         - Magic value from library

.global main
.extern add_numbers
.extern multiply_by_three
.extern double_value
.extern MAGIC_VALUE

; RAM section for test results
.org 0x80000000
    result_add:         .byte 1
    result_multiply:    .byte 1
    result_double:      .byte 1
    result_magic:       .byte 1

; Code section
.org 0x2000
main:
    ; Test add_numbers function
    ; Parameters: L0 = first number, L1 = second number
    ; Return: L0 = result
    ld l0, 0x03
    ld l1, 0x07
    call add_numbers
    st [result_add], l0     ; Should be 0x0A (10)
    
    ; Test multiply_by_three function
    ; Parameter: L0 = number to multiply
    ; Return: L0 = result
    ld l0, 0x07
    call multiply_by_three
    st [result_multiply], l0  ; Should be 0x15 (21)
    
    ; Test double_value function
    ; Parameter: L0 = number
    ; Return: L0 = result
    ld l0, 0x08
    call double_value
    st [result_double], l0  ; Should be 0x10 (16)
    
    ; Test external data symbol
    ld l0, [MAGIC_VALUE]
    st [result_magic], l0   ; Should be 0x2A (42)
    
    ; End program
    stop
