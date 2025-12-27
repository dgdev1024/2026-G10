; Test 19: Library Pattern - Math Library
; Tests: Math utility functions
;
; Provides basic math operations.

.org 0x00002200

.global math_add
.global math_sub
.global math_mul2
.global math_div2

; Function: math_add
; Input: D0 = a, D1 = b
; Output: D0 = a + b
math_add:
    add d0, d1
    ret

; Function: math_sub
; Input: D0 = a, D1 = b
; Output: D0 = a - b
math_sub:
    sub d0, d1
    ret

; Function: math_mul2
; Input: D0 = value
; Output: D0 = value * 2
math_mul2:
    add d0, d0              ; D0 = D0 * 2
    ret

; Function: math_div2
; Input: D0 = value
; Output: D0 = value / 2 (simple right shift via rotate)
math_div2:
    mv d1, d0               ; Copy to D1
    srl l0                  ; Shift L0 (low byte) right
    ret
