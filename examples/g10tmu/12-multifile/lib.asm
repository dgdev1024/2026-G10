; Test 12: Multi-file Project - Library Module
; This module provides utility functions called by the main module.
; Note: G10 ALU/CMP operations use L0/W0/D0 as accumulator.

.global add_numbers
.global multiply_by_three
.global double_value
.global MAGIC_VALUE

; Data section - constant values
.rom
MAGIC_VALUE:
    .byte 0x2A          ; 42 - the answer to everything

; Code section - library functions
; add_numbers: Adds two 8-bit numbers
; Input: L0 = first number, L1 = second number
; Output: L0 = sum
add_numbers:
    add l0, l1
    ret

; multiply_by_three: Multiplies an 8-bit number by 3
; Input: L0 = number
; Output: L0 = number * 3
multiply_by_three:
    ; result = n + n + n
    mv l1, l0           ; L1 = n
    add l0, l1          ; L0 = n + n = 2n
    add l0, l1          ; L0 = 2n + n = 3n
    ret

; double_value: Doubles an 8-bit number
; Input: L0 = number
; Output: L0 = number * 2
double_value:
    mv l1, l0           ; L1 = n
    add l0, l1          ; L0 = n + n = 2n
    ret
