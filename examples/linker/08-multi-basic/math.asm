; Test 08: Multi-File Basic - Math Module
; Tests: Global symbol exports
;
; This module provides the add_numbers function.

.org 0x00002100

; Export the function
.global add_numbers

; Function: add_numbers
; Input: D0 = first number, D1 = second number
; Output: D0 = sum
add_numbers:
    add d0, d1              ; D0 = D0 + D1
    ret
