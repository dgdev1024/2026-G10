; Test 18: Loop with macro expression count
; This test verifies that loop count can be an expression with macros.

.define SIZE 4
.define OFFSET 1

.org 0x0000

.repeat SIZE - OFFSET, i
    .byte {i + OFFSET}
.endrepeat

halt

;----------------------------------------------------------------------------
;
; TEST CASE: Loop with macro expression count
;
; The repeat count is SIZE - OFFSET = 4 - 1 = 3.
; Loop variable i goes 0, 1, 2 and we output i + OFFSET = 1, 2, 3.
;
; EXPECTED OUTPUT:
; .org 0x0000
; .byte 1
; .byte 2
; .byte 3
; halt
;
;----------------------------------------------------------------------------
