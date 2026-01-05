; Test 11: .continue directive
; This test verifies that .continue skips to the next iteration.

.org 0x0000

.for i, 0, 6
    .if i == 2 || i == 4
        .continue
    .endif
    .byte {i}
.endfor

halt

;----------------------------------------------------------------------------
;
; TEST CASE: .continue directive
;
; The loop should skip values 2 and 4, outputting 0, 1, 3, 5.
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte 0
; .byte 1
; .byte 3
; .byte 5
; halt
;
;----------------------------------------------------------------------------
