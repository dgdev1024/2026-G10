; Test 15: .for loop that doesn't execute
; This test verifies that a .for loop with start >= end doesn't execute.

.org 0x0000

.for i, 5, 5
    nop     ; Should NOT appear (start == end)
.endfor

.for j, 10, 5
    nop     ; Should NOT appear (start > end with positive step)
.endfor

halt

;----------------------------------------------------------------------------
;
; TEST CASE: .for loop that doesn't execute
;
; When start >= end (with positive step), the loop body should not execute.
;
; EXPECTED OUTPUT:
; .org 0x0000
; halt
;
;----------------------------------------------------------------------------
