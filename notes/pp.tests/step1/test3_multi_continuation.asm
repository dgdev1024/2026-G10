; Test 3: Multiple Line Continuations
; This test verifies multiple consecutive line continuations.

.org 0x0000

; Multiple consecutive continuations
ld r0, \
    \
    \
    0xFF

; Continuation spanning many lines
add r0, \
    r1, \
    r2, \
    r3

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Multiple Line Continuations
;
; This test verifies that the preprocessor correctly handles multiple
; consecutive line continuations, as well as line continuations that span
; across several lines. All backslash-newline sequences should be removed.
;
; EXPECTED OUTPUT:
;
; .org 0x0000
; ld r0, 0xFF
; add r0, r1, r2, r3
; halt
;
;----------------------------------------------------------------------------
