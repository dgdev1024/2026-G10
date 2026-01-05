; Test 7: .ifndef for undefined macro
; This test verifies that .ifndef works when macro is NOT defined.

; Note: DEBUG is NOT defined

.org 0x0000

.ifndef DEBUG
    ld r0, 0        ; Release mode
.endif

halt

;----------------------------------------------------------------------------
;
; TEST CASE: .ifndef for undefined macro
;
; When the macro is NOT defined, the .ifndef block should be included.
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 0
; halt
;
;----------------------------------------------------------------------------
