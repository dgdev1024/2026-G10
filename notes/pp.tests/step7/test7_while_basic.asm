; Test 7: Basic .while loop
; This test verifies that .while loop executes while condition is true.

.define COUNT 0

.org 0x0000

.while COUNT < 3
    .byte {COUNT}
    .define COUNT {COUNT + 1}
.endwhile

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Basic .while loop
;
; The loop should execute while COUNT < 3, outputting 0, 1, 2.
; Note: COUNT is modified inside the loop using .define redefinition.
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte 0
; .byte 1
; .byte 2
; halt
;
;----------------------------------------------------------------------------
