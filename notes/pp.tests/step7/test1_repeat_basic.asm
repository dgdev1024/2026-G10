; Test 1: Basic .repeat loop
; This test verifies that .repeat executes the body the specified number of times.

.org 0x0000

.repeat 3
    nop
.endrepeat

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Basic .repeat loop
;
; The loop body (nop) should be repeated 3 times.
;
; EXPECTED OUTPUT:
; .org 0x0000
; nop
; nop
; nop
; halt
;
;----------------------------------------------------------------------------
