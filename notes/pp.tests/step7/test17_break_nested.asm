; Test 17: .break in nested loops (only breaks inner)
; This test verifies that .break only affects the innermost loop.

.org 0x0000

.for i, 0, 3
    .for j, 0, 10
        .if j == 2
            .break
        .endif
        .byte {i * 10 + j}
    .endfor
.endfor

halt

;----------------------------------------------------------------------------
;
; TEST CASE: .break in nested loops
;
; The inner loop breaks when j == 2, but outer loop continues.
; For each i (0, 1, 2), j outputs 0, 1 before breaking.
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte 0
; .byte 1
; .byte 10
; .byte 11
; .byte 20
; .byte 21
; halt
;
;----------------------------------------------------------------------------
