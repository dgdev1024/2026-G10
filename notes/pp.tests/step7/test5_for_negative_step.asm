; Test 5: .for loop with negative step
; This test verifies that .for loop works with a negative step.

.org 0x0000

.for i, 10, 0, -2
    .byte {i}
.endfor

halt

;----------------------------------------------------------------------------
;
; TEST CASE: .for loop with negative step
;
; The loop should iterate with i = 10, 8, 6, 4, 2 (step -2, end is exclusive).
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte 10
; .byte 8
; .byte 6
; .byte 4
; .byte 2
; halt
;
;----------------------------------------------------------------------------
