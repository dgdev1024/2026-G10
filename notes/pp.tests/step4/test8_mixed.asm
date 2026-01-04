; Test 8: Mixed Identifier and String Interpolation
; This test verifies both features working together.

.define BASE 100

.org 0x0000

label_{BASE}:
    .byte "Address is {BASE + 0x10}."
    ld r0, {BASE * 2}
    halt

;----------------------------------------------------------------------------
;
; TEST CASE: Mixed Identifier and String Interpolation
;
; This test verifies that identifier interpolation, string interpolation,
; and standalone braced expressions all work together correctly.
;
; EXPECTED OUTPUT:
; .org 0x0000
; label_100:
; .byte "Address is 116."
; ld r0, 200
; halt
;
;----------------------------------------------------------------------------
