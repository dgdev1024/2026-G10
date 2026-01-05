; Test 9: Nested .for loops
; This test verifies that nested loops work correctly.

.org 0x0000

.for i, 0, 2
    .for j, 0, 3
        .byte {i * 3 + j}
    .endfor
.endfor

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Nested .for loops
;
; The outer loop iterates i = 0, 1
; The inner loop iterates j = 0, 1, 2
; This produces values 0, 1, 2, 3, 4, 5
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte 0
; .byte 1
; .byte 2
; .byte 3
; .byte 4
; .byte 5
; halt
;
;----------------------------------------------------------------------------
