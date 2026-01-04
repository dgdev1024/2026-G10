; Test 3: Braced Expression in Middle of Identifier
; This test verifies that braced expressions can appear in the middle.

.org 0x0000

prefix_{10 + 15}_suffix:   ; Should become prefix_25_suffix:
    ld r0, 0
    halt

;----------------------------------------------------------------------------
;
; TEST CASE: Braced Expression in Middle of Identifier
;
; This test verifies that a braced expression in the middle of an
; identifier is evaluated and concatenated with both prefix and suffix.
;
; EXPECTED OUTPUT:
; .org 0x0000
; prefix_25_suffix:
; ld r0, 0
; halt
;
;----------------------------------------------------------------------------
