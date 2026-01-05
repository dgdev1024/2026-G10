; Test 5: .ifdef for undefined macro
; This test verifies that .ifdef works when macro is NOT defined.

; Note: FEATURE_ENABLED is NOT defined

.org 0x0000

.ifdef FEATURE_ENABLED
    ld r0, 1        ; This should NOT appear
.endif

halt

;----------------------------------------------------------------------------
;
; TEST CASE: .ifdef for undefined macro
;
; When the macro is NOT defined, the .ifdef block should be excluded.
;
; EXPECTED OUTPUT:
; .org 0x0000
; halt
;
;----------------------------------------------------------------------------
