; Test 1: Basic .if with true condition
; This test verifies that code inside a true .if block is included.

.define DEBUG 1

.org 0x0000

.if DEBUG
    ld r0, 42
.endif

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Basic .if with true condition
;
; When the condition is true (non-zero), the code inside the .if block
; should be included in the output.
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 42
; halt
;
;----------------------------------------------------------------------------
