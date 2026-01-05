; Test 6: .for loop with custom step
; This test verifies that .for loop works with a custom positive step.

.org 0x0000

.for i, 0, 10, 3
    .byte {i}
.endfor

halt

;----------------------------------------------------------------------------
;
; TEST CASE: .for loop with custom step
;
; The loop should iterate with i = 0, 3, 6, 9 (step 3, end is exclusive).
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte 0
; .byte 3
; .byte 6
; .byte 9
; halt
;
;----------------------------------------------------------------------------
