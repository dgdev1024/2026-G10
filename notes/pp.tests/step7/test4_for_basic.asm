; Test 4: Basic .for loop
; This test verifies that .for loop iterates from start to end-1.

.org 0x0000

.for i, 0, 5
    .byte {i}
.endfor

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Basic .for loop
;
; The loop should iterate with i = 0, 1, 2, 3, 4 (end is exclusive).
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
