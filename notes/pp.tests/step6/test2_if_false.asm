; Test 2: Basic .if with false condition
; This test verifies that code inside a false .if block is excluded.

.define DEBUG 0

.org 0x0000

.if DEBUG
    ld r0, 42       ; This should NOT appear
.endif

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Basic .if with false condition
;
; When the condition is false (zero), the code inside the .if block
; should be excluded from the output.
;
; EXPECTED OUTPUT:
; .org 0x0000
; halt
;
;----------------------------------------------------------------------------
