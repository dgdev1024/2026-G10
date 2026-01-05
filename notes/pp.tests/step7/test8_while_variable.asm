; Test 8: .while loop with loop variable
; This test verifies that .while can use a loop variable for iteration tracking.

.define LIMIT 4

.org 0x0000

.while LIMIT > 0, i
    .byte {i}
    .define LIMIT {LIMIT - 1}
.endwhile

halt

;----------------------------------------------------------------------------
;
; TEST CASE: .while loop with loop variable
;
; The loop variable 'i' tracks iterations (0, 1, 2, 3) while LIMIT counts down.
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte 0
; .byte 1
; .byte 2
; .byte 3
; halt
;
;----------------------------------------------------------------------------
