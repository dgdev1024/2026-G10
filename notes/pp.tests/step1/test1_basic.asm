; Test 1: Basic Token Output
; This test verifies that basic tokens are correctly converted back to
; string output with appropriate spacing.

.org 0x0000

start:
    ld r0, 0x42
    ld r1, r0
    add r0, r1
    halt

;----------------------------------------------------------------------------
;
; TEST CASE: Basic Token Output
;
; This test verifies that the preprocessor correctly converts tokens back
; to a string representation. The output should preserve the structure of
; the original source code, with proper spacing between tokens.
;
; EXPECTED OUTPUT:
;
; .org 0x0000
; start:
; ld r0, 0x42
; ld r1, r0
; add r0, r1
; halt
;
;----------------------------------------------------------------------------
