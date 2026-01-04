; Test 2: Line Continuation
; This test verifies that backslash line continuation works correctly.

.org 0x0000

; Single line continuation
ld r0, \
    0x42

; Another continuation example
add r0, \
    r1

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Line Continuation
;
; This test verifies that the preprocessor correctly handles line
; continuation using the backslash (\) character. When a backslash appears
; at the end of a line (immediately before the newline), both the backslash
; and newline should be removed, joining the lines together.
;
; EXPECTED OUTPUT:
;
; .org 0x0000
; ld r0, 0x42
; add r0, r1
; halt
;
;----------------------------------------------------------------------------
