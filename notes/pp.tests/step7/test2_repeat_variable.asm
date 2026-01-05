; Test 2: .repeat with loop variable
; This test verifies that the loop variable takes values 0, 1, 2, ...

.org 0x0000

.repeat 4, i
    .byte {i}
.endrepeat

halt

;----------------------------------------------------------------------------
;
; TEST CASE: .repeat with loop variable
;
; The loop variable 'i' should take values 0, 1, 2, 3 on each iteration.
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
