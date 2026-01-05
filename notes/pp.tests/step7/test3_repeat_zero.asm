; Test 3: .repeat with count 0
; This test verifies that .repeat with count 0 skips the body entirely.

.org 0x0000

.repeat 0
    nop     ; Should NOT appear
.endrepeat

halt

;----------------------------------------------------------------------------
;
; TEST CASE: .repeat with count 0
;
; When count is 0, the loop body should be skipped entirely.
;
; EXPECTED OUTPUT:
; .org 0x0000
; halt
;
;----------------------------------------------------------------------------
