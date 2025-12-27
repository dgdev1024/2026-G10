; Test 10: Three Modules - Math Module
; Tests: Math functions exported for use by other modules

.org 0x00002200

.global add_values
.global subtract_values
.global multiply_by_two

; Function: add_values
; Input: D0, D1
; Output: D0 = D0 + D1
add_values:
    add d0, d1
    ret

; Function: subtract_values
; Input: D0, D1
; Output: D0 = D0 - D1
subtract_values:
    sub d0, d1
    ret

; Function: multiply_by_two
; Input: D0
; Output: D0 = D0 * 2
multiply_by_two:
    add d0, d0              ; D0 = D0 * 2
    ret
