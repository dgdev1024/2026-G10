; Test 6: .ifndef for defined macro
; This test verifies that .ifndef works when macro IS defined.

.define DEBUG 1

.org 0x0000

.ifndef DEBUG
    ld r0, 0        ; This should NOT appear (DEBUG is defined)
.endif

halt

;----------------------------------------------------------------------------
;
; TEST CASE: .ifndef for defined macro
;
; When the macro IS defined, the .ifndef block should be excluded.
;
; EXPECTED OUTPUT:
; .org 0x0000
; halt
;
;----------------------------------------------------------------------------
