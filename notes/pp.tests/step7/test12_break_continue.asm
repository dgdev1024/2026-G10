; Test 12: .break and .continue together
; This test verifies both .break and .continue work together.

.org 0x0000

.for i, 0, 10
    .if i == 3
        .continue
    .endif
    .if i == 7
        .break
    .endif
    .byte {i}
.endfor

halt

;----------------------------------------------------------------------------
;
; TEST CASE: .break and .continue together
;
; Skip i == 3 (continue), break at i == 7.
; Output: 0, 1, 2, 4, 5, 6
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte 0
; .byte 1
; .byte 2
; .byte 4
; .byte 5
; .byte 6
; halt
;
;----------------------------------------------------------------------------
