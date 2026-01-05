; Test 4: .ifdef for defined macro
; This test verifies that .ifdef works when macro IS defined.

.define FEATURE_ENABLED 1

.org 0x0000

.ifdef FEATURE_ENABLED
    ld r0, 1        ; Feature is enabled
.endif

halt

;----------------------------------------------------------------------------
;
; TEST CASE: .ifdef for defined macro
;
; When the macro is defined, the .ifdef block should be included.
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 1
; halt
;
;----------------------------------------------------------------------------
