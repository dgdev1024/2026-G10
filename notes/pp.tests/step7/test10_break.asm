; Test 10: .break directive
; This test verifies that .break exits the loop early.

.org 0x0000

.for i, 0, 10
    .if i == 5
        .break
    .endif
    .byte {i}
.endfor

halt

;----------------------------------------------------------------------------
;
; TEST CASE: .break directive
;
; The loop should output 0, 1, 2, 3, 4 and then break when i == 5.
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte 0
; .byte 1
; .byte 2
; .byte 3
; .byte 4
; halt
;
;----------------------------------------------------------------------------
