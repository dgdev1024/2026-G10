; Test 8: Chained .elif
; This test verifies .elif chains work correctly.

.define VERSION 2

.org 0x0000

.if VERSION == 1
    ld r0, 1
.elif VERSION == 2
    ld r0, 2
.elif VERSION == 3
    ld r0, 3
.else
    ld r0, 0
.endif

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Chained .elif
;
; When VERSION is 2, only the second branch (.elif VERSION == 2) should
; be taken.
;
; EXPECTED OUTPUT:
; .org 0x0000
; ld r0, 2
; halt
;
;----------------------------------------------------------------------------
