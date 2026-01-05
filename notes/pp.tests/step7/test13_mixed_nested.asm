; Test 13: Mixed loop types nested
; This test verifies different loop types can be nested.

.org 0x0000

.repeat 2, outer
    .for inner, 0, 2
        .byte {outer * 2 + inner}
    .endfor
.endrepeat

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Mixed loop types nested
;
; Outer .repeat loop: outer = 0, 1
; Inner .for loop: inner = 0, 1
; Output: 0, 1, 2, 3
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
